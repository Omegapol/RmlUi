//
// Created by Mateusz Raczynski on 7/20/2021.
//

#include "DataFeedTypeCheck.h"
#include "DataFeed.h"


namespace Rml {
	template<typename Output>
	class TypeCheckerChildrenImpl : public TypeCheckerChildren<Output> {
	public:
		virtual ~TypeCheckerChildrenImpl() = default;

		explicit TypeCheckerChildrenImpl(const Vector <String> &_children) : TypeCheckerChildren<Output>(_children) {};

		UniquePtr <DataFeedBase<Output>> Make(DataVariable data) override {
			return MakeUnique<DataFeed<Output>>(data, std::move(MakeUnique<TypeCheckerChildrenImpl<Output>>(this->children)));
		}

		Output get_sub(DataVariable /*data*/) override {
			// for built in types there is no need for actual implementation here
			return Output();
		}
	};


	impl::TypeCheckersStorageType GetGraphTypes() {
		impl::TypeCheckersStorageType res;
		res[Family<Vector2f>::Id()] = MakeShared<TypeCheckerChildrenImpl<Vector2f>>(Vector<String>{
			"x",
			"y"
		});
		res[Family<CandleStickData>::Id()] = MakeShared<TypeCheckerChildrenImpl<CandleStickData>>(Vector<String>{
			"h",
			"l",
			"en",
			"ex"
		});
		return res;
	}
}
