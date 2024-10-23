#pragma once

#include "Light.h"

#include <UGM/rgb.h>
#include <UGM/point.h>
#include <UGM/vec.h>
#include <UGM/normal.h>

namespace Ubpa {
	class Texture2D;

	// EnvLight's radiance = intensity * color * texture(u, v)
	struct EnvLight : Light 
	{
		[[range("0,")]]
		float intensity;
		rgbf color;
		Texture2D* texture;

		rgbf RadianceFactor() const noexcept { return intensity * color; }
		rgbf Radiance(const pointf2& uv) const noexcept;
		rgbf Radiance(const vecf3& dir) const noexcept;

		// radiance, wi, pdf
		std::tuple<rgbf, vecf3, float> Sample() const noexcept;
		float PDF(const vecf3& dir) const noexcept;

		// radiance, wi, pdf
		// assert(n.is_normalized())
		std::tuple<rgbf, vecf3, float> Sample(const normalf& n) const noexcept;
		float PDF(const vecf3& dir, const normalf& n) const noexcept;

		EnvLight(float intensity = 1.f, const rgbf& color = rgbf{ 1.f,1.f,1.f }, Texture2D* texture = nullptr);

		static void OnRegister();

		size_t w, h;
		size_t N;

		std::vector<double> prob_table;
		std::vector<double> accept;
		std::vector<double> alias;

		void ConstructAliasTable();
		std::tuple<rgbf, vecf3, float> SampleImportance() const noexcept;
		float PDFImportance(const vecf3& dir, const normalf& n) const noexcept;

		void Init();
	};
}
