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

#ifndef RMLUI_CORE_DATASTRUCTHANDLE_H
#define RMLUI_CORE_DATASTRUCTHANDLE_H

#include "Header.h"
#include "Types.h"
#include "Traits.h"
#include "DataTypes.h"
#include "DataVariable.h"


namespace Rml {

template<typename Object>
class StructHandle {
public:
	StructHandle(DataTypeRegister* type_register, StructDefinition* struct_definition, StructDefinition* struct_definition_const) : type_register(type_register), struct_definition(struct_definition),
	 struct_definition_const(struct_definition_const) {}

	/// Register a member object.
	/// @note Underlying type must be registered before it is used as a member.
	/// @note Getter functions can return by reference, raw pointer, or by value. If returned by value,
	///       the returned type must be a scalar data type, otherwise any data type can be used.
	/// @example
	///		struct Invader {
	///			int health;
	/// 	};
	///		struct_handle.RegisterMember("health", &Invader::health);
	template <typename MemberType>
	bool RegisterMember(const String& name, MemberType Object::* member_object_ptr)
	{
		return CreateMemberObjectDefinition(name, member_object_ptr);
	}

	/// Register a member getter function.
	/// @note Underlying type must be registered before it is used as a member.
	/// @note Getter functions can return by reference, raw pointer, or by value. If returned by value,
	///       the returned type must be a scalar data type, otherwise any data type can be used.
	/// @example
	///		struct Invader {
	///			std::vector<Weapon>& GetWeapons();
	///			/* ... */
	/// 	};
	///		struct_handle.RegisterMember("weapons", &Invader::GetWeapons);
	template <typename ReturnType>
	bool RegisterMember(const String& name, ReturnType(Object::* member_get_func_ptr)())
	{
		return RegisterMemberGetter(name, member_get_func_ptr);
	}

	template <typename ReturnType>
	bool RegisterMember(const String& name, ReturnType(Object::* member_get_func_ptr)() const)
	{
		return RegisterMemberGetter(name, member_get_func_ptr);
	}


	/// Register member getter and setter functions. The getter and setter functions must return and assign scalar value types.
	/// @note Underlying type must be registered before it is used as a member.
	/// @note Getter and setter functions can return by reference, raw pointer, or by value.
	/// @example
	///		struct Invader {
	///			MyColorType GetColor() const;
	///			void SetColor(MyColorType color);
	///			/* ... */
	/// 	};
	///		struct_handle.RegisterMember("color", &Invader::GetColor, &Invader::SetColor);
	template <typename ReturnType, typename AssignType>
	bool RegisterMember(const String& name, ReturnType(Object::* member_get_func_ptr)(), void(Object::* member_set_func_ptr)(AssignType))
	{
		using BasicReturnType = typename std::remove_reference<ReturnType>::type;
		using BasicAssignType = typename std::remove_const<typename std::remove_reference<AssignType>::type>::type;
		using UnderlyingType = typename std::conditional<IsVoidMemberFunc<ReturnType>::value, BasicAssignType, BasicReturnType>::type;

		static_assert(IsVoidMemberFunc<ReturnType>::value || IsVoidMemberFunc<AssignType>::value || std::is_same<BasicReturnType, BasicAssignType>::value, "Provided getter and setter functions must get and set the same type.");

		return CreateMemberScalarGetSetFuncDefinition<UnderlyingType>(name, member_get_func_ptr, member_set_func_ptr);
	}

	explicit operator bool() const {
		return type_register && struct_definition;
	}

private:

	// Member getter with reference return type.
	template <typename ReturnType>
	bool RegisterMemberGetter(const String& name, ReturnType& (Object::* member_get_func_ptr)()) {
		return CreateMemberGetFuncDefinition<ReturnType>(name, member_get_func_ptr);
	}

	// Member getter with pointer return type.
	template <typename ReturnType>
	bool RegisterMemberGetter(const String& name, ReturnType* (Object::* member_get_func_ptr)()) {
		return CreateMemberGetFuncDefinition<ReturnType>(name, member_get_func_ptr);
	}

	// Member getter with pointer return type.
	template <typename ReturnType>
	bool RegisterMemberGetter(const String& name, ReturnType* const (Object::* member_get_func_ptr)()) {
		return CreateMemberGetFuncDefinition<ReturnType>(name, member_get_func_ptr);
	}

	// Member getter with value return type, only valid for scalar return types.
	template <typename ReturnType>
	bool RegisterMemberGetter(const String& name, ReturnType(Object::* member_get_func_ptr)()) {
		using BasicReturnType = typename std::remove_reference<ReturnType>::type;
		using SetType = VoidMemberFunc Object::*;
		return CreateMemberScalarGetSetFuncDefinition<BasicReturnType>(name, member_get_func_ptr, SetType{});
	}

	template <typename ReturnType>
	bool RegisterMemberGetter(const String& name, ReturnType& (Object::* member_get_func_ptr)() const) {
		return CreateMemberGetFuncDefinitionConst<ReturnType>(name, member_get_func_ptr);
	}

	template <typename ReturnType>
	bool RegisterMemberGetter(const String& name, ReturnType* (Object::* member_get_func_ptr)() const) {
		return CreateMemberGetFuncDefinitionConst<ReturnType>(name, member_get_func_ptr);
	}

	template <typename ReturnType>
	bool RegisterMemberGetter(const String& name, ReturnType* const (Object::* member_get_func_ptr)() const) {
		return CreateMemberGetFuncDefinitionConst<ReturnType>(name, member_get_func_ptr);
	}

	template <typename ReturnType>
	bool RegisterMemberGetter(const String& name, ReturnType(Object::* member_get_func_ptr)() const) {
		using BasicReturnType = typename std::remove_reference<ReturnType>::type;
		using SetType = VoidMemberFunc Object::*;
		return CreateMemberScalarGetSetFuncDefinitionConst<BasicReturnType>(name, member_get_func_ptr, SetType{});
	}

	template<typename ContainerObject, typename MemberType>
	bool CreateMemberObjectDefinitionImpl(StructDefinition* definition, const String& name, MemberType Object::* member_ptr);

	template<typename MemberType>
	bool CreateMemberObjectDefinition(const String& name, MemberType Object::* member_ptr);

	template<typename BasicReturnType, typename MemberType>
	bool CreateMemberGetFuncDefinition(const String& name, MemberType Object::* member_get_func_ptr);

	template<typename BasicReturnType, typename MemberType>
	bool CreateMemberGetFuncDefinitionConst(const String& name, MemberType Object::* member_get_func_ptr);

	template<typename UnderlyingType, typename MemberGetType, typename MemberSetType>
	bool CreateMemberScalarGetSetFuncDefinitionImpl(StructDefinition* definition, const String& name, MemberGetType Object::* member_get_func_ptr, MemberSetType Object::* member_set_func_ptr);

	template<typename UnderlyingType, typename MemberGetType, typename MemberSetType>
	bool CreateMemberScalarGetSetFuncDefinition(const String& name, MemberGetType Object::* member_get_func_ptr, MemberSetType Object::* member_set_func_ptr);

	template<typename UnderlyingType, typename MemberGetType, typename MemberSetType>
	bool CreateMemberScalarGetSetFuncDefinitionConst(const String& name, MemberGetType Object::* member_get_func_ptr, MemberSetType Object::* member_set_func_ptr);

	DataTypeRegister* type_register;
	StructDefinition* struct_definition;
	StructDefinition* struct_definition_const;
};



template<typename Object>
template<typename ContainerObject, typename MemberType>
bool StructHandle<Object>::CreateMemberObjectDefinitionImpl(StructDefinition* definition, const String& name, MemberType Object::* member_ptr)
{
	VariableDefinition *underlying_definition = type_register->GetDefinition<MemberType, ContainerObject>();
	if (!underlying_definition)
		return false;
	definition->AddMember(
			name,
			MakeUnique<MemberObjectDefinition<ContainerObject, MemberType>>(underlying_definition, member_ptr)
	);
	return true;
}


template<typename Object>
template<typename MemberType>
bool StructHandle<Object>::CreateMemberObjectDefinition(const String& name, MemberType Object::* member_ptr)
{
	return  CreateMemberObjectDefinitionImpl<Object, MemberType>(struct_definition, name, member_ptr) &&
			CreateMemberObjectDefinitionImpl<const Object, const MemberType>(struct_definition_const, name, member_ptr);
}

template<typename Object>
template<typename BasicReturnType, typename MemberType>
bool StructHandle<Object>::CreateMemberGetFuncDefinition(const String& name, MemberType Object::* member_get_func_ptr)
{
	VariableDefinition* underlying_definition = type_register->GetDefinition<BasicReturnType, Object>();
	if (!underlying_definition)
		return false;
	struct_definition->AddMember(
			name,
			MakeUnique<MemberGetFuncDefinition<Object, MemberType, BasicReturnType>>(underlying_definition, member_get_func_ptr)
	);
	return true;
}

template<typename Object>
template<typename BasicReturnType, typename MemberType>
bool StructHandle<Object>::CreateMemberGetFuncDefinitionConst(const String &name,
															  MemberType Object::* member_get_func_ptr) {

	VariableDefinition *underlying_definition = type_register->GetDefinition<BasicReturnType, Object>();
	if (!underlying_definition)
		return false;
	struct_definition->AddMember(
			name,
			MakeUnique<MemberGetFuncDefinition<Object, MemberType, BasicReturnType>>(underlying_definition,
																					 member_get_func_ptr)
	);
	VariableDefinition *underlying_definition_const = type_register->GetDefinition<BasicReturnType, Object>();
	if (!underlying_definition_const)
		return false;
	struct_definition_const->AddMember(
			name,
			MakeUnique<MemberGetFuncDefinition<const Object, MemberType, BasicReturnType>>(
					underlying_definition_const, member_get_func_ptr)
	);
	return true;
}

template<typename Object>
template<typename UnderlyingType, typename MemberGetType, typename MemberSetType>
bool StructHandle<Object>::CreateMemberScalarGetSetFuncDefinition(const String& name, MemberGetType Object::* member_get_func_ptr, MemberSetType Object::* member_set_func_ptr)
{
	static_assert(std::is_default_constructible<UnderlyingType>::value, "Struct member getter/setter functions must return/assign a type that is default constructible.");

	return 	CreateMemberScalarGetSetFuncDefinitionImpl<UnderlyingType>(struct_definition, name, member_get_func_ptr, member_set_func_ptr);
}

template<typename Object>
template<typename UnderlyingType, typename MemberGetType, typename MemberSetType>
bool StructHandle<Object>::CreateMemberScalarGetSetFuncDefinitionConst(const String& name, MemberGetType Object::* member_get_func_ptr, MemberSetType Object::* member_set_func_ptr)
{
	static_assert(std::is_default_constructible<UnderlyingType>::value, "Struct member getter/setter functions must return/assign a type that is default constructible.");

	return 	CreateMemberScalarGetSetFuncDefinitionImpl<UnderlyingType>(struct_definition, name, member_get_func_ptr, member_set_func_ptr)
	        &&
		    CreateMemberScalarGetSetFuncDefinitionImpl<UnderlyingType>(struct_definition_const, name, member_get_func_ptr, member_set_func_ptr);
}


template<typename Object>
template<typename UnderlyingType, typename MemberGetType, typename MemberSetType>
bool StructHandle<Object>::CreateMemberScalarGetSetFuncDefinitionImpl(StructDefinition* definition, const String& name, MemberGetType Object::* member_get_func_ptr, MemberSetType Object::* member_set_func_ptr)
{
	static_assert(std::is_default_constructible<UnderlyingType>::value, "Struct member getter/setter functions must return/assign a type that is default constructible.");

	VariableDefinition* underlying_definition = type_register->GetDefinition<UnderlyingType, Object>();
	if (!underlying_definition)
		return false;

	if (!IsVoidMemberFunc<MemberGetType>::value)
	{
		RMLUI_ASSERTMSG(member_get_func_ptr, "Expected member getter function, but none provided.");
	}
	if (!IsVoidMemberFunc<MemberSetType>::value)
	{
		RMLUI_ASSERTMSG(member_get_func_ptr, "Expected member setter function, but none provided.");
	}

	if (underlying_definition->Type() != DataVariableType::Scalar)
	{
		RMLUI_LOG_TYPE_ERROR(UnderlyingType, "Returning or assigning a non-scalar variable by value in a data struct member function is illegal. Only scalar data variables are allowed here: A getter function returning by value, or a getter/setter function pair.");
		return false;
	}

	definition->AddMember(
			name,
			MakeUnique<MemberScalarGetSetFuncDefinition<Object, MemberGetType, MemberSetType, UnderlyingType>>(underlying_definition, member_get_func_ptr, member_set_func_ptr)
	);
	return true;
}
} // namespace Rml
#endif
