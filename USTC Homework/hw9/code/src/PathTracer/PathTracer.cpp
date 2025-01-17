#include "PathTracer.h"

#include <UBL/Image.h>

#include <iostream>

#include <thread>




using namespace Ubpa;
using namespace std;

PathTracer::PathTracer(const Scene* scene, const SObj* cam_obj, Image* img, size_t spp)
	: scene{ scene },
	bvh{ const_cast<Scene*>(scene) },
	img{ img },
	cam{ cam_obj->Get<Cmpt::Camera>() },
	ccs{ cam->GenCoordinateSystem(cam_obj->Get<Cmpt::L2W>()->value) },
	spp{ spp }
{
	IntersectorVisibility::Instance();
	IntersectorClosest::Instance();

	scene->Each([this](const Cmpt::Light* light) ->bool {
		if (!vtable_is<EnvLight>(light->light.get()))
			return true; // continue

		env_light = static_cast<EnvLight*>(light->light.get());
		return false; // stop
	});

	// TODO: preprocess env_light here
	env_light->Init();


}

void PathTracer::Run() {
	img->SetAll(0.f);

#ifdef NDEBUG
	const size_t core_num = std::thread::hardware_concurrency();
	auto work = [this, core_num](size_t id) {
		for (size_t j = id; j < img->height; j += core_num) 
		{
			for (size_t i = 0; i < img->width; i++) 
			{
				//for each pixel (i,j)
				for (size_t k = 0; k < spp; k++) 
				{
					//spp is the sample of pixel
					float u = (i + rand01<float>() - 0.5f) / img->width;
					float v = (j + rand01<float>() - 0.5f) / img->height;
					//generate ray from the pixel
					rayf3 r = cam->GenRay(u, v, ccs);
					rgbf Lo;
					//Trading
					do { Lo = Shade(IntersectorClosest::Instance().Visit(&bvh, r), -r.dir, true); }
					while (Lo.has_nan());
					img->At<rgbf>(i, j) += Lo ;
				}
				img->At<rgbf>(i, j) /= float(spp);
			}
			float progress = (j + 1) / float(img->height);
			cout << progress << endl;


		}
	};
	vector<thread> workers;
	for (size_t i = 0; i < core_num; i++)
		workers.emplace_back(work, i);
	for (auto& worker : workers)
		worker.join();
#else
	for (size_t j = 0; j < img->height; j++) {
		for (size_t i = 0; i < img->width; i++) {
			for (size_t k = 0; k < spp; k++) {
				float u = (i + rand01<float>() - 0.5f) / img->width;
				float v = (j + rand01<float>() - 0.5f) / img->height;
				rayf3 r = cam->GenRay(u, v, ccs);
				rgbf Lo;
				do { Lo = Shade(IntersectorClosest::Instance().Visit(&bvh, r), -r.dir, true); }
				while (Lo.has_nan());
				img->At<rgbf>(i, j) += Lo / static_cast<float>(spp);
			}
		}
		float progress = (j + 1) / float(img->height);
		cout << progress << endl;
	}
#endif



	Denoise();

	std::cout << "PathTracing finish!" << std::endl;

}

rgbf PathTracer::Shade(const IntersectorClosest::Rst& intersection, const vecf3& wo, bool last_bounce_specular) const {
	// TODO: HW9 - Trace
	// [ Tips ]
	// - EnvLight::Radiance(<direction>), <direction> is pointing to environment light
	// - AreaLight::Radiance(<uv>)
	// - rayf3: point, dir, tmin, **tmax**
	// - IntersectorVisibility::Instance().Visit(&bvh, <rayf3>)
	//   - tmin = EPSILON<float>
	//   - tmax = distance to light - EPSILON<float>
	// - IntersectorCloest::Instance().Visit(&bvh, <rayf3>)
	//   - tmin as default (EPSILON<float>)
	//   - tmax as default (FLT_MAX)
	//
	// struct IntersectorClosest::Rst {
	//	 bool IsIntersected() const noexcept { return sobj != nullptr; }
	//	 const SObj* sobj{ nullptr }; // intersection sobj
	//	 pointf3 pos; // intersection point's position
	//	 pointf2 uv; // texcoord
	//   normalf n; // normal, normalized
	//	 vecf3 tangent; // perpendicular to normal, normalized
	// };

	constexpr rgbf error_color = rgbf{ 1.f,0.f,1.f };
	constexpr rgbf todo_color = rgbf{ 0.f,1.f,0.f };
	constexpr rgbf zero_color = rgbf{ 0.f,0.f,0.f };

	if (!intersection.IsIntersected()) {
		if (last_bounce_specular && env_light != nullptr) 
		{
			// TODO: environment light
			return env_light->Radiance(-wo);
		}
		else
			return zero_color;
	}
	
	if (!intersection.sobj->Get<Cmpt::Material>()) {
		auto light = intersection.sobj->Get<Cmpt::Light>();
		if(!light) return error_color;

		if (last_bounce_specular) { // avoid double-count
			auto area_light = dynamic_cast<const AreaLight*>(light->light.get());
			if (!area_light) return error_color;
			return area_light->color;
			// TODO: area light

			return area_light->Radiance(intersection.uv);
		}else
			return zero_color;
	}
	
	rgbf L_dir{ 0.f };
	rgbf L_indir{ 0.f };

	vecf3 pos = intersection.pos.cast_to<vecf3>();
	scene->Each([=, &L_dir](const Cmpt::Light* light, const Cmpt::L2W* l2w, const Cmpt::SObjPtr* ptr) {
		// TODO: L_dir += ...
		// - use PathTracer::BRDF to get BRDF value
		SampleLightResult sample_light_rst = SampleLight(intersection, wo, light, l2w, ptr);
		if (sample_light_rst.pd <= 0)
			return;
		if (sample_light_rst.is_infinity) 
		{
			// TODO: L_dir of environment light
			// - only use SampleLightResult::L, n, pd
			// - SampleLightResult::x is useless
			vecf3 wi = -sample_light_rst.n.cast_to<vecf3>();
			bool visible = IntersectorVisibility::Instance().Visit(&bvh, rayf3(intersection.pos, wi));
			if (visible)
			{
				float cosxy = wi.cos_theta(intersection.n.cast_to<vecf3>());
				L_dir += sample_light_rst.L * BRDF(intersection, wi, wo) * abs(cosxy) / sample_light_rst.pd;
			}
		}
		else {
			// TODO: L_dir of area light
			vecf3 x_to_y = sample_light_rst.x.cast_to<vecf3>() - pos;
			vecf3 wi = x_to_y.normalize();
			float dist = x_to_y.norm();
			bool visible = IntersectorVisibility::Instance().Visit(&bvh, rayf3(intersection.pos, wi, EPSILON<float>, dist- EPSILON<float>));
			if (visible)
			{
				if (wi.dot(sample_light_rst.n.cast_to<vecf3>())<0.f)
				{
					float cosxy = wi.cos_theta(intersection.n.cast_to<vecf3>());
					float cosyx=-wi.cos_theta(sample_light_rst.n.cast_to<vecf3>());
					float geo = abs(cosxy) * abs(cosyx) / (dist * dist);
					L_dir += sample_light_rst.L * BRDF(intersection, wi, wo)*geo/sample_light_rst.pd;
				}
			}
		}
	});


	// TODO: Russian Roulette
	// - rand01<float>() : random in [0, 1)

	float pdf_RR = 0.1f;
	if (rand01<float>() < pdf_RR)
		return L_dir;

	// TODO: recursion
	// - use PathTracer::SampleBRDF to get wi and pd (probability density)
	// wi may be **under** the surface
	// - use PathTracer::BRDF to get BRDF value

	auto wi_pd = SampleBRDF(intersection, wo);
	vecf3 wi = get<0>(wi_pd);
	while (wi.dot(intersection.n.cast_to<vecf3>()) < 0.0f)
	{
		wi_pd = SampleBRDF(intersection, wo);
		wi = get<0>(wi_pd);
	}

	float pd = get<1>(wi_pd);
	auto next_intersection = IntersectorClosest::Instance().Visit(&bvh, rayf3(intersection.pos, wi));
	rgbf Li = Shade(next_intersection, -wi, false);

	L_indir += BRDF(intersection, wi, wo) * Li* abs(wi.cos_theta(intersection.n.cast_to<vecf3>())) / pd;

	// TODO: combine L_dir and L_indir

	return (L_indir+L_dir); // you should commemt this line
}

PathTracer::SampleLightResult PathTracer::SampleLight(const IntersectorClosest::Rst& intersection, const vecf3& wo, const Cmpt::Light* light, const Cmpt::L2W* l2w, const Cmpt::SObjPtr* ptr) const {
	PathTracer::SampleLightResult rst;

	auto mat = intersection.sobj->Get<Cmpt::Material>();
	if (!mat) return rst; // invalid
	auto brdf = dynamic_cast<const stdBRDF*>(mat->material.get());
	if (!brdf) return rst; // not support

	if (wo.dot(intersection.n.cast_to<vecf3>()) < 0)
		return rst;

	rgbf albedo = brdf->Albedo(intersection.uv);
	float metalness = brdf->Metalness(intersection.uv);
	float roughness = brdf->Roughness(intersection.uv);
	//          roughness    0     0.5     1
	// metalness----------------------------
	//     0    |           0.5    0.38    0
	//    0.5   |           0.75   0.56    0
	//     1    |            1     0.75    0
	float p_mat = (1 + metalness) / 2 * (1 - stdBRDF::Alpha(roughness)); // 0 - 1

	auto w2l = l2w->value->inverse();

	float pd_mat, pd_light; // dwi / dA
	vecf3 wi;
	vecf3 light_wi; // wi in light space

	// multi-importance sampling, MIS
	// multi-importance sampling, MIS

	if (vtable_is<AreaLight>(light->light.get())) {
		// [1] area light

		auto area_light = static_cast<const AreaLight*>(light->light.get());
		auto geo = ptr->value->Get<Cmpt::Geometry>();
		if (!geo) return rst; // invalid
		if (!vtable_is<Square>(geo->primitive.get())) return rst; // not support

		rst.n = (l2w->value * normalf{ 0,1,0 }).normalize();
		auto light_p = w2l * intersection.pos; // intersection point's position in light space
		scalef3 world_s = l2w->WorldScale();
		float area = world_s[0] * world_s[1] * Square::area;

		if (rand01<float>() < p_mat) {
			// [1.1] sample material

			// pd_mat : dwi
			tie(wi, pd_mat) = SampleBRDF(intersection, wo);
			light_wi = (w2l * wi).normalize(); // wi in light space

			auto light_r = rayf3{ light_p, light_wi, -std::numeric_limits<float>::max() }; // ray in light space
			auto [isIntersected, t, xz] = light_r.intersect_std_square();
			if (isIntersected) {

				pointf3 p_on_light = pointf3{ xz[0], 0.f, xz[1] };

				pd_light = 1 / area;

				rst.x = l2w->value * p_on_light;
				rst.L = area_light->Radiance({ (xz[0] + 1) / 2, (1 - xz[1]) / 2 });

				// pd_mat : dw -> dA
				float dist2 = light_p.distance2(p_on_light);
				float cos_theta_l = (-light_wi)[1];
				pd_mat *= std::abs(cos_theta_l) / dist2;
			}
			else {
				pd_light = 0.f;
				rst.L = 0.f;
				rst.x = 0.f;
			}
		}
		else {
			// [1.2] sample area light

			auto Xi = uniform_in_square<float>(); // [0, 1] x [0, 1]
			pointf3 p_on_light{ 2 * Xi[0] - 1, 0, 2 * Xi[1] - 1 }; // light space
			vecf3 diff = p_on_light - light_p;
			float dist2 = diff.norm2();
			light_wi = diff / std::sqrt(dist2);
			wi = (l2w->value * light_wi).normalize();

			pd_light = 1.f / area;

			rst.L = area_light->Radiance(Xi.cast_to<pointf2>());
			rst.x = l2w->value * p_on_light;
			rst.n = l2w->UpInWorld().cast_to<normalf>();

			// pd_mat : dw
			matf3 surface_to_world = svecf::TBN(intersection.n.cast_to<vecf3>(), intersection.tangent);
			matf3 world_to_surface = surface_to_world.inverse();
			svecf s_wo = (world_to_surface * wo).cast_to<svecf>();
			svecf s_wi = (world_to_surface * wi).cast_to<svecf>();
			pd_mat = brdf->PDF(albedo, metalness, roughness, s_wi, s_wo);

			// pd_mat : dw -> dA
			float cos_theta_l = (-light_wi)[1];
			pd_mat *= std::abs(cos_theta_l) / dist2;
		}
	}
	else if (vtable_is<EnvLight>(light->light.get())) {
		// [2] env light
		auto env_light = static_cast<const EnvLight*>(light->light.get());
		auto light_n = (w2l * intersection.n).normalize(); // intersetion point's normal in light space

		rst.is_infinity = true;
		rst.x = std::numeric_limits<float>::max();

		if (rand01<float>() < p_mat) {
			tie(wi, pd_mat) = SampleBRDF(intersection, wo);
			light_wi = (w2l * wi).normalize();
			rst.L = env_light->Radiance(light_wi);
			// pd_light : dwi
			pd_light = env_light->PDF(light_wi, light_n); // TODO: use your PDF
			//pd_light = env_light->PDFImportance(light_wi, light_n); // TODO: use your PDF
		}
		else {
			// pd_light : dwi
			tie(rst.L, light_wi, pd_light) = env_light->Sample(light_n); // TODO: use your sampling method
			//tie(rst.L, light_wi, pd_light) = env_light->SampleImportance(); // TODO: use your sampling method
			wi = (l2w->value * light_wi).normalize();
			matf3 surface_to_world = svecf::TBN(intersection.n.cast_to<vecf3>(), intersection.tangent);
			matf3 world_to_surface = surface_to_world.inverse();
			svecf s_wo = (world_to_surface * wo).cast_to<svecf>();
			svecf s_wi = (world_to_surface * wi).cast_to<svecf>();
			pd_mat = brdf->PDF(albedo, metalness, roughness, s_wi, s_wo);
		}

		rst.n = -wi.cast_to<normalf>();
	}
	else
		return rst; // not support

			
	rst.pd = p_mat * pd_mat + (1 - p_mat) * pd_light;
	
	return rst;
}

std::tuple<vecf3, float> PathTracer::SampleBRDF(const IntersectorClosest::Rst& intersection, const vecf3& wo) {
	auto mat = intersection.sobj->Get<Cmpt::Material>();
	if (!mat) return { vecf3{0.f}, 0.f };
	auto brdf = dynamic_cast<const stdBRDF*>(mat->material.get());
	if (!brdf) return { vecf3{0.f}, 0.f };

	matf3 surface_to_world = svecf::TBN(intersection.n.cast_to<vecf3>(), intersection.tangent);
	matf3 world_to_surface = surface_to_world.inverse();
	svecf s_wo = (world_to_surface * wo).cast_to<svecf>();

	rgbf albedo = brdf->Albedo(intersection.uv);
	float metalness = brdf->Metalness(intersection.uv);
	float roughness = brdf->Roughness(intersection.uv);

	auto [s_wi, pdf] = brdf->Sample(albedo, metalness, roughness, s_wo);
	if (pdf == 0.f)
		return { vecf3{0.f}, 0.f };

	vecf3 wi = surface_to_world * s_wi;

	return { wi,pdf };
}

rgbf PathTracer::BRDF(IntersectorClosest::Rst intersection, const vecf3& wi, const vecf3& wo) {
	auto mat = intersection.sobj->Get<Cmpt::Material>();
	if (!mat) return rgbf{ 1.f,0.f,1.f };
	auto brdf = dynamic_cast<const stdBRDF*>(mat->material.get());
	if (!brdf) return rgbf{ 1.f,0.f,1.f };

	matf3 surface_to_world = svecf::TBN(intersection.n.cast_to<vecf3>(), intersection.tangent);
	matf3 world_to_surface = surface_to_world.inverse();
	svecf s_wi = (world_to_surface * wi).cast_to<svecf>();
	svecf s_wo = (world_to_surface * wo).cast_to<svecf>();

	rgbf albedo = brdf->Albedo(intersection.uv);
	float metalness = brdf->Metalness(intersection.uv);
	float roughness = brdf->Roughness(intersection.uv);

	return brdf->BRDF(albedo, metalness, roughness, s_wi, s_wo);
}


void PathTracer::Denoise()
{
	//denoise1
	
	for (size_t j = 1; j < img->height - 1; j++)
	{
		for (size_t i = 1; i < img->width - 1; i++)
		{
			//for each pixel (i,j)
			float mean = 0.f, var = 0.f;
			for (int i1 = -1; i1 <= 1; i1++)
			{
				for (int j1 = -1; j1 <= 1; j1++)
				{
					if (i1 == 0 && j1 == 0)
						continue;
					mean += img->At<rgbf>(i + i1, j + j1).illumination();
					var += img->At<rgbf>(i + i1, j + j1).illumination() * img->At<rgbf>(i + i1, j + j1).illumination();
				}
			}

			mean /= 8.f;
			var /= 8.f;
			var = var - mean * mean;

			if ((img->At<rgbf>(i, j).illumination() - mean) * (img->At<rgbf>(i, j).illumination() - mean) > 3 * var)
				img->At<rgbf>(i, j) *= mean / img->At<rgbf>(i, j).illumination();
		}


	}

//denoise2
	
	/*float lambda = 0.3;
	rgbf center_old{ 0.f }, center_new{ 0.f }; size_t N = 0;
	for (size_t j = 1; j < img->height - 1; j++)
	{
		for (size_t i = 1; i < img->width - 1; i++)
		{
			//for each pixel (i,j)
			rgbf delta{ 0.f };
			center_old += img->At<rgbf>(i, j);

			for (int i1 = -1; i1 <= 1; i1++)
			{
				for (int j1 = -1; j1 <= 1; j1++)
				{
					if (i1 == 0 && j1 == 0)
					{
						continue;
					}
					else
						delta += img->At<rgbf>(i + i1, j + j1) / 8;
				}
			}

			float d = (img->At<rgbf>(i, j)[0] - delta[0]) * (img->At<rgbf>(i, j)[0] - delta[0]) +
				(img->At<rgbf>(i, j)[1] - delta[1]) * (img->At<rgbf>(i, j)[1] - delta[1]) +
				(img->At<rgbf>(i, j)[2] - delta[2]) * (img->At<rgbf>(i, j)[2] - delta[2]);

			if (d > 20)
				img->At<rgbf>(i, j) = delta;

		}
		
	}*/


}