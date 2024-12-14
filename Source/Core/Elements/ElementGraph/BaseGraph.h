//
// Created by Mateusz Raczynski on 4/13/2021.
//

#ifndef RMLUI_BASEGRAPH_H
#define RMLUI_BASEGRAPH_H

#include "RmlUi/Core/Geometry.h"
#include "RmlUi/Core/ComputedValues.h"
#include "RmlUi/Core/Elements/DataFeedBase.h"
#include "DataFeedTransforms.h"
#include "RmlUi/Core/Elements/ElementCanvasDrawable.h"
#include "RmlUi/Core/Elements/ElementDataSink.h"
#include "../common.h"
#include <limits>


namespace Rml {

	struct GraphRenderMetadata {
		unsigned int vertices_delta = 0;
		unsigned int indices_delta = 0;
		unsigned int target_size = 0;
	};

	template <typename DataFeed>
	struct CachedGraphData {
		Rml::SharedPtr<typename DataFeed::Chunk> chunk;
		Rml::SharedPtr<Geometry> geometry;
	};

	template <typename InputType>
	class BaseGraphImpl : public ElementCanvasDrawable, public ElementDataSink, public Element {
		using DataFeedType = DataFeedBase<InputType>;

	protected:
		// The geometry used to render this element.
		bool geometry_dirty;
		UniquePtr<DataFeedType> feed;

		Rml::UnorderedMap<double, CachedGraphData<DataFeedType>> data;
		Rml::Vector<double> active_data;

		bool reset_all = true;
	public:
		virtual ~BaseGraphImpl();

		BaseGraphImpl(const String &tag);

		// ElementCanvasDrawable interface
		void RenderOnCanvas(Vector2f canvas_size, Vector2f transl) override;

		void GenerateGeometry(Vector2f canvasSize) override;

		void DirtyCanvas() override;

		Vector2f GetCanvasSize() override;

		// ElementDataSink interface
		bool FeedData(DataVariable data) override;

		// Feed dependant
		double GetInterval();

	public: //todo: protected?
		void Render(Vector2f translation);

	protected:
		bool DoDataTransform(DataVariable data);

		template<typename StartType>
		bool DoDataTransform(UniquePtr<DataFeedBase<StartType>> new_feed);

		void OnStyleSheetChange() override;

		void OnAttributeChange(const ElementAttributes &changed_attributes) override;

		void OnPropertyChange(const PropertyIdSet &changed_properties) override;

		virtual float GetWidth();

		virtual void GenerateGeometryInit();
		virtual void GenerateGeometryStart(const InputType &from);
		virtual GraphRenderMetadata GenerateGeometryPart(Vertex *vertices, int *indices,
										  const InputType &next,
										  int index_offset, Colourb colour,
										  Vector2f offset, Rml::Vector2f scale,
										  double width) = 0;


		virtual GraphRenderMetadata GetMetadata(unsigned int input_size) = 0;
	};

	template <typename InputType>
	void BaseGraphImpl<InputType>::GenerateGeometry(Vector2f canvasSize) {
		//todo: remove canvas, just read parent
		if (HasAttribute("static") && HasAttribute("data-source"))
			Log::Message(Log::LT_ERROR, "Graph has two or more data sources defined!");
		if (HasAttribute("static")) {
			auto static_data = GetAttribute("static")->template Get<String>();
			UniquePtr<DataFeedBase<Vector2f>> new_ptr = std::move(TransformFeedString<Vector2f>(static_data));
			DoDataTransform(std::move(new_ptr));
		}
		if (!feed)
			return;

		if (this->reset_all) {
			this->data.clear();
		}

		auto x_viewed = view.x;

		auto __vec = feed->GetData(x_viewed); //todo: visibility use here
		if (!__vec.size())
			return;

		auto &first_vec = __vec[0]->data; //todo: do we need chunks?

		if (!first_vec.size())
			return;

		auto first_idx = __vec[0]->start - 1;
		if (first_idx == (unsigned int) -1)
			first_idx = 0;
		auto first = feed->GetElement(first_idx)[0]->data[0];
		auto ratios = GetRatios(canvasSize);

		const ComputedValues &computed = GetComputedValues();
		float width = this->GetWidth();

		float opacity = computed.opacity();
		Colourb quad_colour = computed.color();
		quad_colour.alpha = (byte) (opacity * (float) quad_colour.alpha);

		this->GenerateGeometryInit();

		auto prev = first;
		bool prev_found = false;
		this->active_data.clear();
		unsigned int vec_no = 0;
		for (auto &v: __vec) {
			++vec_no;
			auto &vec = v->data;
			auto found = this->data.find(v->base);

			CachedGraphData<DataFeedType> cache;

			if (found != this->data.end()) {
				cache = found->second;
				auto chunk = cache.chunk;
				if (chunk == v && !this->reset_all &&
					vec_no != __vec.size()) {
					this->active_data.push_back(v->base);
					prev = vec[vec.size() - 1];
					prev_found = true;
					continue;
				} else {
					cache.chunk = v;
					cache.geometry = Rml::MakeShared<Geometry>();
				}
			} else {
				cache.chunk = v;
				cache.geometry = Rml::MakeShared<Geometry>();
			}

			if (prev_found)
				GenerateGeometryStart(prev);

			auto man = this->GetRenderManager();
			auto mesh = Mesh();

			Vector <Vertex> &vertices = mesh.vertices;
			Vector<int> &indices = mesh.indices;

			// Release the old geometry before specifying the new vertices.
			RMLUI_ASSERT(vec.size() < std::numeric_limits<unsigned int>::max());
			GraphRenderMetadata meta = GetMetadata(static_cast<unsigned int>(vec.size()));

			// generate only if there is anything to be parsed
			if (meta.target_size != 0) {
				vertices.resize(meta.target_size * meta.vertices_delta);
				indices.resize(meta.target_size * meta.indices_delta);


				unsigned int idx = 0;
				unsigned int vert_count = 0;
				for (size_t i = 0; i < vec.size(); ++i) {
					auto cur = vec[i];
					auto used_meta = GenerateGeometryPart(&vertices[vert_count], &indices[idx], cur,
													 vert_count, quad_colour,
													 {0.0f, 0.0f}, ratios, width);
					idx += used_meta.indices_delta;
					vert_count += used_meta.vertices_delta;
					prev = cur;
				}
			}

			this->active_data.push_back(v->base);
			this->data[v->base] = cache;

			prev_found = false;
			*cache.geometry = man->MakeGeometry(std::move(mesh));
		}
		reset_all = false;
		geometry_dirty = false;
	}

	template <typename InputType>
	BaseGraphImpl<InputType>::BaseGraphImpl(const String &tag): Element(tag) {

	}

	template <typename InputType>
	void BaseGraphImpl<InputType>::Render(Vector2f translation) {
//		// Render the geometry beginning at this element's content region.
		for (auto active_id: active_data) {
			auto dt_it = data.find(active_id);
			if (dt_it == data.end()) {
				Log::Message(Log::LT_DEBUG, "Invalid data");
				continue;
			}
			auto &dt = dt_it->second;
			auto &geometry = *(dt.geometry);
			Vector2f offset = GetOffset(GetCanvasSize());
			geometry.Render(translation + offset);
		}
	}

	template <typename InputType>
	void BaseGraphImpl<InputType>::RenderOnCanvas(Vector2f canvas_size, Vector2f transl) {
		if (geometry_dirty)
			GenerateGeometry(canvas_size);
		Render(transl);
	}

	template <typename InputType>
	bool BaseGraphImpl<InputType>::FeedData(DataVariable data) {
		if (feed == nullptr) {
			DoDataTransform(data);
		}

		geometry_dirty = true;
		return true;
	}

	template <typename InputType>
	bool BaseGraphImpl<InputType>::DoDataTransform(DataVariable data) {

		auto transf_attrib = GetAttribute("transform");
		auto transf = String("");
		if(transf_attrib)
			transf = transf_attrib->template Get<String>();

		DataFeedAggArguments args;
		{
			auto arg_attrib = GetAttribute("transf-step");
			auto arg = 0.0;
			if(arg_attrib)
				arg = arg_attrib->template Get<double>();
			args.agg_size = arg;
		}

		feed = std::move(MakeFeed<InputType>(data, transf, args));

		if(!feed)
		{
			Log::Message(Log::LT_ERROR, "Feed is in invalid state!");
		}
		return true;
	}

	template <typename InputType>
	template <typename StartType>
	bool BaseGraphImpl<InputType>::DoDataTransform(UniquePtr<DataFeedBase<StartType>> new_feed) {
		auto transf_attrib = GetAttribute("transform");
		auto transf = String("");
		if(transf_attrib)
			transf = transf_attrib->template Get<String>();
		feed = std::move(TransformFeed<InputType>(std::move(new_feed), transf, DataFeedAggArguments()));

		geometry_dirty = true;
		return true;
	}

	template <typename InputType>
	BaseGraphImpl<InputType>::~BaseGraphImpl() {

	}

	template <typename InputType>
	void BaseGraphImpl<InputType>::OnStyleSheetChange() {
		Element::OnStyleSheetChange();
		DirtyCanvas();
	}

	template <typename InputType>
	void BaseGraphImpl<InputType>::OnAttributeChange(const ElementAttributes &changed_attributes) {
		Element::OnAttributeChange(changed_attributes);
		DirtyCanvas();
	}

	template <typename InputType>
	void BaseGraphImpl<InputType>::OnPropertyChange(const PropertyIdSet &changed_properties) {
		//todo: don't generate, just dirty
		geometry_dirty = true;
		Element::OnPropertyChange(changed_properties);
		DirtyCanvas();
	}

	template <typename InputType>
	Vector2f BaseGraphImpl<InputType>::GetCanvasSize() {
		auto parent = GetParentNode();
		Vector2f quad_size = parent->GetBox().GetSize(BoxArea::Content).Round();
		return quad_size;
	}

	template <typename InputType>
	float BaseGraphImpl<InputType>::GetWidth() {
		const ComputedValues &computed = GetComputedValues();
		float width = 2;
		auto computed_width = computed.width();
		if (computed_width.type == Style::Width::Length && computed_width.value != 0) {
			width = computed_width.value;
		}
		return width;
	}

	template <typename InputType>
	void BaseGraphImpl<InputType>::DirtyCanvas() {
		reset_all = true;
	}

	template <typename InputType>
	void BaseGraphImpl<InputType>::GenerateGeometryInit() {

	}

	template <typename InputType>
	void BaseGraphImpl<InputType>::GenerateGeometryStart(const InputType & /*from*/) {}

	template <typename InputType>
	double BaseGraphImpl<InputType>::GetInterval() {
		auto res = feed->GetInterval();
		if(res)
			return res;
		auto interval_attr = this->GetAttribute("interval");
		auto interval = 5;
		if (interval_attr) {
			interval = interval_attr->template Get<int>();
		}
		return interval;
	}

}

#endif //RMLUI_BASEGRAPH_H
