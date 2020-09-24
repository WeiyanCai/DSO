//
// Created by cwy on 2020/9/24.
//

#include "CuParameterBlock.h"

#include "FullSystem/HessianBlocks.h"

namespace dso {
	PointParameterBlock::PointParameterBlock(raw_data_type* raw) : ParameterBlock {raw} {}

	PointParameterBlock::~PointParameterBlock() = default;

	FrameParameterBlock::FrameParameterBlock(raw_data_type* raw) : ParameterBlock {raw} {}

	FrameParameterBlock::~FrameParameterBlock() = default;

//	void FrameParameterBlock::update() {
//		Eigen::Map<Vec8>(parameters_prior_.data(), 8, 1) = raw_->getPrior().head<8>();
//	}
}