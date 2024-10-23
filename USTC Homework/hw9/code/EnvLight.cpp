#include <UScene/core/Light/EnvLight.h>

#include <UScene/core/Material/Texture2D.h>

#include "detail/dynamic_reflection/EnvLight.inl"

#include <UGM/sample.h>

#include<queue>


using namespace Ubpa;
using namespace std;

EnvLight::EnvLight(float intensity, const rgbf& color, Texture2D* texture)
	: intensity{ intensity }, color{ color }, texture{ texture }
{
}

rgbf EnvLight::Radiance(const pointf2& uv) const noexcept {
	if (!texture)
		return RadianceFactor();

	return RadianceFactor() * texture->Sample(uv).to_rgb();
}

rgbf EnvLight::Radiance(const vecf3& dir) const noexcept {
	return RadianceFactor() * Radiance(dir.normalize().cast_to<normalf>().to_sphere_texcoord());
}

tuple<rgbf, vecf3, float> EnvLight::Sample() const noexcept 
{
	vecf3 wi = uniform_on_sphere<float>().cast_to<vecf3>();
	return { Radiance(wi), wi, 1 / (4 * PI<float>) };
}

float EnvLight::PDF(const vecf3& dir) const noexcept {
	return 1 / (4 * PI<float>);
}


tuple<rgbf, vecf3, float> EnvLight::Sample(const normalf& n) const noexcept {
	assert(n.is_normalized());
	auto vn = n.cast_to<vecf3>();
	svecf s_wi = cos_weighted_on_hemisphere<float>().cast_to<svecf>();
	vecf3 t = 0.f;
	t[n.min_dim()] = 1.f;
	t[n.max_dim()] = 1.f;
	t = (t - t.dot(vn) * vn).normalize();
	vecf3 b = vn.cross(t);
	vecf3 wi = svecf::TBN(vn, t) * s_wi;
	return { Radiance(wi), wi, s_wi.cos_stheta() / PI<float> };
}

float EnvLight::PDF(const vecf3& dir, const normalf& n) const noexcept {
	float cos_theta = dir.cos_theta(n.cast_to<vecf3>());
	return cos_theta / PI<float>;
}



void EnvLight::OnRegister() {
	detail::dynamic_reflection::ReflRegister_EnvLight();
}

void EnvLight::Init() 
{
	ConstructAliasTable();
}



void EnvLight::ConstructAliasTable()
{
	prob_table.clear();
	accept.clear();
	alias.clear();

	w = texture->img->width;
	h = texture->img->height;
	N = w * h;
	std::vector<size_t> smaller,larger;

	prob_table.resize(N);
	accept.resize(N);
	alias.resize(N);
	
	double sum = 0;
	for (size_t x = 0; x < w; x++)
	{
		for (size_t y = 0; y < h; y++)
		{
			size_t idx = x + y * w;
			prob_table[idx] = texture->img->At(x, y).to_rgb().illumination();

			sum += prob_table[idx];
			alias[idx] = -1;

		}
	}

	for (size_t x = 0; x < w; x++)
	{
		for (size_t y = 0; y < h; y++)
		{
			size_t idx = x *h+y;
			prob_table[idx] = prob_table[idx] / sum;
			accept[idx] = prob_table[idx] * N;

			if (accept[idx] < 1)
			{
				smaller.push_back(idx);
			}
			else if (accept[idx] > 1)
			{
				larger.push_back(idx);
			}


		}

	}



	while ((!smaller.empty()) && (!larger.empty()) )
	{

		size_t smallerone = smaller.back();
		size_t largerone = larger.back();

		//fill
		alias[smallerone] = largerone;
		accept[largerone] = accept[largerone] - (1 - accept[smallerone]);
		smaller.pop_back();
		
		if (abs(accept[largerone]-1)<1e-10)
		{
			larger.pop_back();
			alias[largerone] = largerone;
		}
		else if (accept[largerone] < 1)
		{
			larger.pop_back();
			smaller.push_back(largerone);
		}

	}

	while (!smaller.empty())
	{
		size_t i = smaller.back();
		alias[i] = i;
		smaller.pop_back();
	}

	while (!larger.empty())
	{
		size_t i = larger.back();
		alias[i] = i;
		larger.pop_back();
	}



}

tuple<rgbf, vecf3, float> EnvLight::SampleImportance() const noexcept
{
	// alias sample
	size_t i = randi() % N; 
	if (rand01<float>() >accept[i])
		i = alias[i];
	if (i == -1) cout<<"Error in alias sampling at:"<<i<<endl;

	float x = (i / h) / (float)w;
	float y = (i % h) / (float)h;

	float phi = x * 2 * PI<float>;
	float theta = (1 - y) * PI<float>;

	vecf3 wi = { cos(phi) * sin(theta) ,cos(theta), sin(phi) * sin(theta) };
	return { Radiance(wi), wi, N* prob_table[i] / (2 * PI<float> *PI<float> *sin(theta) + EPSILON<float>) };

}

float EnvLight::PDFImportance(const vecf3& dir, const normalf& n) const noexcept
{
	normalf ndir = dir.normalize().cast_to<normalf>();
	pointf2 uv = ndir.to_sphere_texcoord();
	size_t i = clamp((size_t)(round((double)uv[0] * w - 0.5)), (size_t)0, w - 1);
	size_t j = clamp((size_t)(round((double)uv[0] * h - 0.5)), (size_t)0, h - 1);
	float theta = PI<float> *(1 - (float)j / h);
	return N * prob_table[i + j * w] / (2 * PI<float> *PI<float> *sin(theta));

}  