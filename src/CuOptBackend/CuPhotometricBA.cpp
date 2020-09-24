//
// Created by cwy on 2020/9/24.
//

#include "CuPhotometricBA.h"

#include "CuParameterBlock.h"

#include "FullSystem/Residuals.h"

namespace dso {
	PhotometricResidual::PhotometricResidual(raw_residual_type* residual)
			: raw_ {residual} {}

	PhotometricBA::PhotometricBA() : adjoint_ {}, residual_blocks_ {} {}

	auto PhotometricBA::point_parameter_blocks() -> point_param_blocks_type {
		return point_param_blocks_;
	}

	auto PhotometricBA::frame_parameter_blocks() -> frame_param_blocks_type {
		return frame_param_blocks_;
	}

	auto PhotometricBA::residual_blocks() -> residual_blocks_type {
		return residual_blocks_;
	}
}