//
// Created by Mateusz Raczynski on 5/19/2021.
//

#ifndef RMLUI_DATAFEEDCONVERTER_H
#define RMLUI_DATAFEEDCONVERTER_H

#include "DataFeed.h"

namespace Rml {
	template<typename InputType, typename OutputType>
	class DataConverter
	{
	public:
		virtual OutputType Convert(InputType& input) = 0;
	};

	template<typename InputType, typename OutputType, typename Func>
	class DataConverterFunc: public DataConverter<InputType, OutputType>
	{
	private:
		Func func;
	public:
		DataConverterFunc(Func foo) {
			func = foo;
		}

		OutputType Convert(InputType& input) override {
			return func(input);
		};
	};

	template<typename InputType, typename OutputType>
	class DataFeedConverter: public DataFeed<OutputType> {
	public:
		DataFeedConverter(UniquePtr<DataFeed<InputType>> input_ptr, UniquePtr<DataConverter<InputType, OutputType>> conv_ptr) {
			input = std::move<input_ptr>();
			conv = std::move<conv_ptr>();
		}
		DataFeedConverter(String input_var, UniquePtr<DataConverter<InputType, OutputType>> conv_ptr) {
			input = MakeUnique<DataFeed<InputType>>(input_var);
			conv = std::move<conv_ptr>();
		}

		OutputType &Get(int i) override {
			InputType& el = input->Get(i);
			OutputType out = conv->Convert(el);
			return out;
		}

	protected:
		UniquePtr<DataFeed<InputType>> input;
		UniquePtr<DataConverter<InputType, OutputType>> conv;
	};
}

#endif //RMLUI_DATAFEEDCONVERTER_H
