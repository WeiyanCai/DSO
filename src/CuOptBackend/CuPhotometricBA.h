//
// Created by cwy on 2020/9/24.
//

#pragma once

#include "util/NumType.h"

namespace dso {
	class PointFrameResidual;

	class FrameParameterBlock;

	class PointParameterBlock;

	template<typename T>
	struct AdjMat {
		T host;
		T target;
	};

	class PhotometricResidual {
	public:
		using raw_residual_type = PointFrameResidual;

	public:
		explicit PhotometricResidual(raw_residual_type* residual);

	private:
		raw_residual_type* raw_;

		FrameParameterBlock const* host_;
		FrameParameterBlock const* target_;
		PointParameterBlock const* point_;
	};

	class PhotometricBA {
	public:
		using mat88_type = Mat88;
		using adj_type = AdjMat<mat88_type>;

		using point_param_block_type = PointParameterBlock;
		using frame_param_block_type = FrameParameterBlock;

		using point_param_blocks_type = std::vector<PointParameterBlock>;
		using frame_param_blocks_type = std::vector<FrameParameterBlock>;

		using residual_block_type = PhotometricResidual;
		using residual_blocks_type = std::vector<residual_block_type>;

	public:
		PhotometricBA();

	public:
		auto point_parameter_blocks() -> point_param_blocks_type;

		auto frame_parameter_blocks() -> frame_param_blocks_type;

		auto residual_blocks() -> residual_blocks_type;

	private:
		adj_type adjoint_;

		point_param_blocks_type point_param_blocks_;
		frame_param_blocks_type frame_param_blocks_;
		residual_blocks_type    residual_blocks_;
	};
}
