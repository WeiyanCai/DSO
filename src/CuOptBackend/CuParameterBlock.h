//
// Created by cwy on 2020/9/24.
//

#pragma once

#include "util/NumType.h"

namespace dso {
	class FrameHessian;

	class PointHessian;

	template<int32_t Dim, typename Raw>
	class ParameterBlock {
	public:
		using real_type = double;
		using sint_type = int32_t;

		using raw_data_type = Raw;

	public:
		explicit ParameterBlock(raw_data_type* raw)
				: raw_ {raw},
				  parameters_prior_ {0},
				  parameters_curr_ {0},
				  parameters_backup_ {0},
				  parameters_delta_ {0} {}

		~ParameterBlock() = default;

	public:
		auto dimension() const -> sint_type {
			return Dim;
		}

	public:
//		virtual void update() = 0;

	protected:
		raw_data_type* raw_;
		std::array<real_type, Dim> parameters_prior_;
		std::array<real_type, Dim> parameters_curr_;
		std::array<real_type, Dim> parameters_backup_;
		std::array<real_type, Dim> parameters_delta_;
	};

//	using PointParameterBlock = ParameterBlock<1, PointHessian>;
//
//	using FrameParameterBlock = ParameterBlock<8, FrameHessian>;


	class PointParameterBlock : public ParameterBlock<1, PointHessian> {
	public:
		using raw_data_type = ParameterBlock::raw_data_type;

	public:
		explicit PointParameterBlock(raw_data_type* raw);

		~PointParameterBlock();

	};

	class FrameParameterBlock : public ParameterBlock<8, FrameHessian> {
	public:
		using raw_data_type = ParameterBlock::raw_data_type;

	public:
		explicit FrameParameterBlock(raw_data_type* raw);

		~FrameParameterBlock();

	public:
//		void update() override;
	};
}
