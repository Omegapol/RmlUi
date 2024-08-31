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

#ifndef RMLUI_CORE_DATADEFINITIONS_H
#define RMLUI_CORE_DATADEFINITIONS_H

#include "Header.h"
#include "Types.h"
#include <type_traits>

namespace Rml {

class VariableDefinition;
class DataTypeRegister;
class TransformFuncRegister;
class DataModelHandle;
class DataVariable;

using DataGetFunc = Function<void(Variant&)>;
using DataSetFunc = Function<void(const Variant&)>;
using DataTransformFunc = Function<bool(Variant&, const VariantList&)>;
using DataEventFunc = Function<void(DataModelHandle, Event&, const VariantList&)>;

template<typename T> using MemberGetFunc = void(T::*)(Variant&);
template<typename T> using MemberSetFunc = void(T::*)(const Variant&);

template<typename T> using DataTypeGetFunc = void(*)(const T*, Variant&);
template<typename T> using DataTypeSetFunc = void(*)(T*, const Variant&);

template<typename Object, typename ReturnType> using MemberGetterFunc = ReturnType(Object::*)();
template<typename Object, typename AssignType> using MemberSetterFunc = void(Object::*)(AssignType);

using DirtyVariables = SmallUnorderedSet<String>;

struct DataAddressEntry {
	DataAddressEntry(String name) : name(name), index(-1) { }
	DataAddressEntry(int index) : index(index) { }
	String name;
	int index;
};
using DataAddress = Vector<DataAddressEntry>;

// DataPointer is a wrapper around void* which preserves the constness of the underlying object.
class DataPointer {
public:
	DataPointer(std::nullptr_t) : ptr(nullptr), cptr(nullptr) {};
	DataPointer(void* p) : ptr(p), cptr(p) {};
	DataPointer(const void* p) : ptr(nullptr), cptr(p) {};

	template<typename T, typename std::enable_if<!std::is_const<typename std::remove_pointer<T>::type>::value, int>::type = 0>
	T Get() {
		return static_cast<T>(ptr);
	}
	template<typename T, typename std::enable_if<std::is_const<typename std::remove_pointer<T>::type>::value, int>::type = 0>
	T Get() {
		return static_cast<T>(cptr);
	}

	operator bool() const {
		return ptr != nullptr || cptr != nullptr;
	}
private:
	void* ptr = nullptr;
	const void* cptr = nullptr;
};

template<class T>
struct PointerTraits {
	using is_pointer = std::false_type;
	using element_type = T;
	// Dereference() function intentionally missing.
};
template<class T>
struct PointerTraits<T*> {
	using is_pointer = std::true_type;
	using element_type = T;
	static DataPointer Dereference(DataPointer ptr) {
		return DataPointer(*ptr.Get<T**>());
	}
};
template<class T>
struct PointerTraits<T* const> {
	using is_pointer = std::true_type;
	using element_type = T;
	static DataPointer Dereference(DataPointer ptr) {
		return DataPointer(*ptr.Get<T* const*>());
	}
};
template<class T>
struct PointerTraits<UniquePtr<T>> {
	using is_pointer = std::true_type;
	using element_type = T;
	static DataPointer Dereference(DataPointer ptr) {
		return DataPointer(ptr.Get<UniquePtr<T>*>()->get());
	}
};
template<class T>
struct PointerTraits<UniquePtr<T> const> {
	using is_pointer = std::true_type;
	using element_type = T;
	static DataPointer Dereference(DataPointer ptr) {
		return DataPointer(ptr.Get<UniquePtr<T> const* const>()->get());
	}
};
template<class T>
struct PointerTraits<SharedPtr<T> const> {
	using is_pointer = std::true_type;
	using element_type = T;
	static DataPointer Dereference(DataPointer ptr) {
		return DataPointer(ptr.Get<SharedPtr<T> const* const>()->get());
	}
};
template<class T>
struct PointerTraits<SharedPtr<T>> {
	using is_pointer = std::true_type;
	using element_type = T;
	static DataPointer Dereference(DataPointer ptr) {
		return DataPointer(ptr.Get<SharedPtr<T>*>()->get());
	}
};

struct VoidMemberFunc {};
template<typename T> using IsVoidMemberFunc = std::is_same<T, VoidMemberFunc>;




#define RMLUI_LOG_TYPE_ERROR(T, msg) RMLUI_ERRORMSG((String(msg) + String("\nT: ") + String(rmlui_type_name<T>())).c_str())
#define RMLUI_LOG_TYPE_ERROR_ASSERT(T, val, msg) RMLUI_ASSERTMSG((val), (String(msg) + String("\nT: ") + String(rmlui_type_name<T>())).c_str())


} // namespace Rml
#endif
