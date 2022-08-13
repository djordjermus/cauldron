#include "control.h"
#pragma once
namespace cauldron::gui {
	class anchoredControl : public control {
	public:
		anchoredControl();
		
		// GETTERS

		common::bounds2<f32> getAnchor() const;
		common::bounds2<f32> getOffset() const;
		common::bounds2<f32> getMargins() const;

		// SETTERS

		virtual void setAnchor(const common::bounds2<f32>& new_anchor);
		//virtual void setAnchor(anchorPreset);
		virtual void setOffset(const common::bounds2<f32>& new_offset);
		virtual void setMargins(const common::bounds2<f32>& new_margins);

		void setBounds(const common::bounds2<i32>& new_bounds) override;

		void recalculateBounds();

		static common::bounds2<i32> calculateBounds(
			const common::bounds2<i32>& frame,
			const common::bounds2<f32>& anchor,
			const common::bounds2<f32>& offset);
		static common::bounds2<i32> boundsToOffset(
			const common::bounds2<i32>& frame,
			const common::bounds2<f32>& anchor,
			const common::bounds2<f32>& bounds);
	private:

		void recalcBoundsOnParentSize(control& sender, sizeData& e);
		void recalcBoundsOnParentSizing(control& sender, sizingData& e);
		void recalcBoundsOnParentChanged(control& sender, changeParentData& e);

		common::bounds2<f32> _anchor;
		common::bounds2<i32> _offset;
	};
}