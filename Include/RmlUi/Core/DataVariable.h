/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019 The RmlUi Team, and contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef RMLUI_CORE_DATAVARIABLE_H
#define RMLUI_CORE_DATAVARIABLE_H

#include "Header.h"
#include "Types.h"
#include "Traits.h"
#include "Variant.h"
#include "DataTypes.h"
#include <iterator>

namespace Rml {

enum class DataVariableType { Scalar, Array, Struct };


/*
*   A 'DataVariable' wraps a user handle (pointer) and a VariableDefinition.
*
*   Together they can be used to get and set variables between the user side and data model side.
*/

class RMLUICORE_API DataVariable {
public:
	DataVariable() {}
	DataVariable(VariableDefinition* definition, VariablePointer ptr) : definition(definition), ptr(ptr) {}

	explicit operator bool() const { return definition; }

	bool Get(Variant& variant);
	bool Set(const Variant& variant);
	int Size();
	DataVariable Child(const DataAddressEntry& address);
	DataVariableType Type();

private:
	VariableDefinition* definition = nullptr;
	VariablePointer ptr = (void*) nullptr;
};


/*
*   A 'VariableDefinition' specifies how a user handle (pointer) is translated to and from a value in the data model.
* 
*   Generally, Scalar types can set and get values, while Array and Struct types can retrieve children based on data addresses.
*/

class RMLUICORE_API VariableDefinition {
public:
	virtual ~VariableDefinition() = default;
	DataVariableType Type() const { return type; }

	virtual bool Get(VariablePointer ptr, Variant& variant);
	virtual bool Set(VariablePointer ptr, const Variant& variant);

	virtual int Size(VariablePointer ptr);
	virtual DataVariable Child(VariablePointer ptr, const DataAddressEntry& address);

protected:
	VariableDefinition(DataVariableType type) : type(type) {}

private:
	DataVariableType type;
};


RMLUICORE_API DataVariable MakeLiteralIntVariable(int value);


template<typename T>
class ScalarDefinition final : public VariableDefinition {
public:
	ScalarDefinition() : VariableDefinition(DataVariableType::Scalar) {}

	bool Get(VariablePointer ptr, Variant& variant) override
	{
		variant = *static_cast<const T*>(
		        VariablePointerGetter<void, T>::Get(ptr)
		        );
		return true;
	}
	bool Set(VariablePointer ptr, const Variant& variant) override
	{
		return variant.GetInto<T>(*static_cast<T*>(
				VariablePointerGetter<void, T>::Get(ptr)
				));
	}
};

class FuncDefinition final : public VariableDefinition {
public:
	FuncDefinition(DataGetFunc get, DataSetFunc set) : VariableDefinition(DataVariableType::Scalar), get(std::move(get)), set(std::move(set)) {}

	bool Get(VariablePointer /*ptr*/, Variant& variant) override
	{
		if (!get)
			return false;
		get(variant);
		return true;
	}
	bool Set(VariablePointer /*ptr*/, const Variant& variant) override
	{
		if (!set)
			return false;
		set(variant);
		return true;
	}

private:
	DataGetFunc get;
	DataSetFunc set;
};

template<typename T>
class ScalarFuncDefinition final : public VariableDefinition {
public:
	ScalarFuncDefinition(DataTypeGetFunc<T> get, DataTypeSetFunc<T> set) : VariableDefinition(DataVariableType::Scalar), get(get), set(set) {}

	bool Get(VariablePointer ptr, Variant& variant) override
	{
		if (!get)
			return false;
		get(static_cast<const T*>(ptr), variant);
		return true;
	}
	bool Set(VariablePointer ptr, const Variant& variant) override
	{
		if (!set)
			return false;
		set(static_cast<T*>(ptr), variant);
		return true;
	}

private:
	DataTypeGetFunc<T> get;
	DataTypeSetFunc<T> set;
};

template<typename Container>
class ArrayDefinition final : public VariableDefinition {
public:
	ArrayDefinition(VariableDefinition* underlying_definition) : VariableDefinition(DataVariableType::Array), underlying_definition(underlying_definition) {}

	int Size(VariablePointer ptr) override {
		return int(static_cast<Container*>(VariablePointerGetter<void, Container>::Get(ptr))->size());
	}

protected:
	DataVariable Child(VariablePointer void_ptr, const DataAddressEntry& address) override
	{
		Container* ptr = static_cast<Container*>(
				VariablePointerGetter<void, Container>::Get(void_ptr)
		);
		const int index = address.index;

		const int container_size = int(ptr->size());
		if (index < 0 || index >= container_size)
		{
			if (address.name == "size")
				return MakeLiteralIntVariable(container_size);

			Log::Message(Log::LT_WARNING, "Data array index out of bounds.");
			return DataVariable();
		}

		auto it = ptr->begin();
		std::advance(it, index);

		VariablePointer next_ptr = &(*it);
		return DataVariable(underlying_definition, next_ptr);
	}

private:
	VariableDefinition* underlying_definition;
};

class RMLUICORE_API BasePointerDefinition : public VariableDefinition {
public:
	BasePointerDefinition(VariableDefinition* underlying_definition) : VariableDefinition(underlying_definition->Type()), underlying_definition(underlying_definition) {}

	bool Get(VariablePointer ptr, Variant& variant) override;
	bool Set(VariablePointer ptr, const Variant& variant) override;
	int Size(VariablePointer ptr) override;
	DataVariable Child(VariablePointer ptr, const DataAddressEntry& address) override;

protected:
	virtual VariablePointer DereferencePointer(VariablePointer ptr) = 0;

private:
	VariableDefinition* underlying_definition;
};

template<typename T>
class PointerDefinition final : public BasePointerDefinition {
public:
	PointerDefinition(VariableDefinition* underlying_definition) : BasePointerDefinition(underlying_definition) {}

protected:
	VariablePointer DereferencePointer(VariablePointer ptr) override
	{
		return PointerTraits<T>::Dereference(ptr);
	}
};


class StructDefinition final : public VariableDefinition {
public:
	StructDefinition() : VariableDefinition(DataVariableType::Struct)
	{}

	DataVariable Child(VariablePointer ptr, const DataAddressEntry& address) override
	{
		const String& name = address.name;
		if (name.empty())
		{
			Log::Message(Log::LT_WARNING, "Expected a struct member name but none given.");
			return DataVariable();
		}

		auto it = members.find(name);
		if (it == members.end())
		{
			Log::Message(Log::LT_WARNING, "Member %s not found in data struct.", name.c_str());
			return DataVariable();
		}

		VariableDefinition* next_definition = it->second.get();

		return DataVariable(next_definition, ptr);
	}

	void AddMember(const String& name, UniquePtr<VariableDefinition> member)
	{
		RMLUI_ASSERT(member);
		bool inserted = members.emplace(name, std::move(member)).second;
		RMLUI_ASSERTMSG(inserted, "Member name already exists.");
		(void)inserted;
	}

private:
	SmallUnorderedMap<String, UniquePtr<VariableDefinition>> members;
};



template<typename Object, typename MemberType>
class MemberObjectDefinition final : public BasePointerDefinition {
public:
	MemberObjectDefinition(VariableDefinition* underlying_definition, MemberType Object::* member_ptr) : BasePointerDefinition(underlying_definition), member_ptr(member_ptr) {}

protected:
	VariablePointer DereferencePointer(VariablePointer base_ptr) override
	{
		auto& var = (static_cast<Object*>(
							 VariablePointerGetter<void, Object>::Get(base_ptr)
					 )->*member_ptr);
		return &var;
	}

private:
	MemberType Object::* member_ptr;
};


template<typename Object, typename MemberType, typename BasicReturnType>
class MemberGetFuncDefinition final : public BasePointerDefinition {
public:

	MemberGetFuncDefinition(VariableDefinition* underlying_definition, MemberType Object::* member_get_func_ptr)
		: BasePointerDefinition(underlying_definition), member_get_func_ptr(member_get_func_ptr)
	{
		using MemberFunc = MemberType Object::*;
		static_assert(std::is_member_function_pointer<MemberFunc>::value, "Must be a member function pointer");
	}

protected:
	VariablePointer DereferencePointer(VariablePointer base_ptr) override
	{
		return static_cast<VariablePointer>(Extract((static_cast<Object*>(
                     VariablePointerGetter<void, Object>::Get(base_ptr)
		        )->*member_get_func_ptr)()));
	}

private:
	// Pointer return types
	BasicReturnType* Extract(BasicReturnType* value) {
		return value;
	}
	// Reference return types
	BasicReturnType* Extract(BasicReturnType& value) {
		return &value;
	}

	MemberType Object::* member_get_func_ptr;
};

template<typename Object, typename MemberGetType, typename MemberSetType, typename UnderlyingType>
class MemberScalarGetSetFuncDefinition final : public VariableDefinition {
public:
	MemberScalarGetSetFuncDefinition(VariableDefinition* underlying_definition, MemberGetType Object::* member_get_func_ptr, MemberSetType Object::* member_set_func_ptr)
		: VariableDefinition(underlying_definition->Type()), underlying_definition(underlying_definition), member_get_func_ptr(member_get_func_ptr), member_set_func_ptr(member_set_func_ptr)
	{
		using MemberGetFunctionPtr = MemberGetType Object::*;
		static_assert(std::is_member_function_pointer<MemberGetFunctionPtr>::value, "Must be a member function pointer");
	}

	bool Get(VariablePointer ptr, Variant& variant) override
	{
		return GetDetail(ptr, variant);
	}
	bool Set(VariablePointer ptr, const Variant& variant) override
	{
		return SetDetail(ptr, variant);
	}

private:
	template<typename T = MemberGetType, typename std::enable_if<std::is_null_pointer<T>::value, int>::type = 0>
	bool GetDetail(VariablePointer /*ptr*/, Variant& /*variant*/)
	{
		return false;
	}

	template<typename T = MemberGetType, typename std::enable_if<!std::is_null_pointer<T>::value, int>::type = 0>
	bool GetDetail(VariablePointer ptr, Variant& variant)
	{
		if (!member_get_func_ptr)
			return false;

		// TODO: Does this work for pointers?
		auto&& value = (static_cast<Object*>(
				VariablePointerGetter<void, Object>::Get(ptr)
				)->*member_get_func_ptr)();
		bool result = underlying_definition->Get(static_cast<VariablePointer>(&value), variant);
		return result;
	}

	template<typename T = MemberSetType, typename std::enable_if<std::is_null_pointer<T>::value, int>::type = 0>
	bool SetDetail(VariablePointer /*ptr*/, const Variant& /*variant*/)
	{
		return false;
	}

	template<typename T = MemberSetType, typename std::enable_if<!std::is_null_pointer<T>::value, int>::type = 0>
	bool SetDetail(VariablePointer ptr, const Variant& variant)
	{
		if (!member_set_func_ptr)
			return false;

		UnderlyingType result;
		if (!underlying_definition->Set(static_cast<VariablePointer>(&result), variant))
			return false;

		(static_cast<Object*>(ptr)->*member_set_func_ptr)(result);

		return true;
	}

	VariableDefinition* underlying_definition;
	MemberGetType Object::* member_get_func_ptr;
	MemberSetType Object::* member_set_func_ptr;
};


} // namespace Rml
#endif
