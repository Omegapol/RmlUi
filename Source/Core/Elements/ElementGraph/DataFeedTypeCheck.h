//
// Created by Mateusz Raczynski on 7/20/2021.
//

#ifndef RMLUI_DATAFEEDTYPECHECK_H
#define RMLUI_DATAFEEDTYPECHECK_H

#include <RmlUi/Core/DataVariable.h>
#include "MakeFeed.h"

namespace Rml {
	namespace impl {
		class TypeCheckerBase {
		private:
			friend void *
			MakeFeedImpl(DataVariable var, FamilyId target_id, const String &transform_str, DataFeedAggArguments args);

			virtual void *get_raw(DataVariable /*data*/) {
				return nullptr;
			};

			virtual void release_raw(void *ptr) = 0;

		public:
			virtual ~TypeCheckerBase() = default;

			virtual void GetId(FamilyId &output) {
				output = (FamilyId) -1;
			}
		};

		typedef UnorderedMap<FamilyId, SharedPtr<TypeCheckerBase>>
				TypeCheckersStorageType;
	}

	void RegisterGraphType(impl::TypeCheckersStorageType &storage, SharedPtr<impl::TypeCheckerBase> factory);

	impl::TypeCheckersStorageType GetGraphTypes();

	template<typename Output>
	class TypeChecker : public impl::TypeCheckerBase {
	private:
		void release_raw(void *ptr) override {
			auto nptr = (DataFeedBase<Output> *) ptr;
			delete nptr;
		}

		void *get_raw(DataVariable data) override {
			auto ptr = get(data);
			return ptr.release();
		}

	public:
		virtual ~TypeChecker() = default;

		void GetId(FamilyId &output) override {
			output = Family<Output>::Id();
		}

		virtual UniquePtr<DataFeedBase<Output>> get(DataVariable data) = 0;

		virtual Output get_sub(DataVariable data) = 0;
	};

	template<typename Output>
	class TypeCheckerChildren : public TypeChecker<Output> {
	private:
		void *get_raw(DataVariable data) override {
			auto ptr = get(data);
			return ptr.release();
		}

	protected:
		const Vector<String> children;

	public:
		virtual ~TypeCheckerChildren() = default;

		explicit TypeCheckerChildren(const Vector<String> &_children) : children(_children) {
		}

		void GetId(FamilyId &output) override {
			output = Family<Output>::Id();
		}

		virtual UniquePtr<DataFeedBase<Output>> Make(DataVariable data) = 0;

		UniquePtr<DataFeedBase<Output>> get(DataVariable data) override {
			auto var = data.Child(1);
			if (!var)
				return UniquePtr<DataFeedBase<Output >>();

			for (const auto &val: children) {
				if (!var.Child(val))
					return UniquePtr<DataFeedBase<Output >>();
			}
			return Make(data);
		}
	};
}
#endif //RMLUI_DATAFEEDTYPECHECK_H
