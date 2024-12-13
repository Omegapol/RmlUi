/*
 * This source file is part of RmlUi, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://github.com/mikke89/RmlUi
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 * Copyright (c) 2019-2023 The RmlUi Team, and contributors
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

#define RMLUI_UNITTESTS_ENABLED

#include "../Common/TestsInterface.h"

#include "../Common/TestsShell.h"
#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <../../RmlUi/Source/Core/DataModel.h>
#include <cmath>
#include <doctest.h>

using namespace Rml;

namespace {

static const String document_rml = R"(
<rml>
<head>
	<title>Test</title>
	<link type="text/template" href="/assets/window.rml"/>
	<style>
		body.window
		{
			left: 50px;
			right: 50px;
			top: 30px;
			bottom: 30px;
			max-width: none;
			max-height: none;
		}
		div#content
		{
			text-align: left;
			padding: 50px;
			box-sizing: border-box;
		}
	</style>
</head>

<body template="window">
<div data-model="basics">

<input type="text" data-value="arrays.x0[1]"/>

<h1>WrapperWrap</h1>
<p>{{ wrapperwrap.i0.val }}</p>
<p>{{ wrapperwrap.i1.val }}</p>
<p>{{ wrapperwrap.i2.val }}</p>
<p>{{ wrapperwrap.i3.val }}</p>
<p>{{ wrapperwrap.x0.val }}</p>
<p>{{ wrapperwrap.x1.val }}</p>
<p>{{ wrapperwrap.x2.val }}</p>
<p>{{ wrapperwrap.x3.val }}</p>

<h1>Globals</h1>
<p>{{ i0 }}</p>
<p>{{ i1 }}</p>
<p>{{ i2 }}</p>
<p>{{ i3 }}</p>
<p>{{ x1 }}</p>
<p>{{ x2 }}</p>
<p>{{ x3.val }}</p>
<p>{{ x4.val }}</p>

<p>{{ s0 }}</p>
<p>{{ s1 }}</p>
<p>{{ s2.val }}</p>
<p>{{ s3.val }}</p>
<p>{{ s4.val }}</p>
<p>{{ s5.val }}</p>
<p>{{ simple }}</p>
<p>{{ scoped }}</p>

<h1>Basic</h1>
<p>{{ basic.a }}</p>
<p>{{ basic.b }}</p>
<p>{{ basic.c }}</p>
<p>{{ basic.d }}</p>
<p>{{ basic.e }}</p>
<p>{{ basic.f }}</p>
<p id="simple" data-event-click="basic.simple = 2">{{ basic.simple }}</p>
<p>{{ basic.scoped }}</p>
<p>{{ basic.x0 }}</p>
<p>{{ basic.x1 }}</p>
<p>{{ basic.x2 }}</p>
<p>{{ basic.x3 }}</p>
<p>{{ basic.x4 }}</p>
<p>{{ basic.x5 }}</p>
<p>{{ basic.x6 }}</p>
<p>{{ basic.x7 }}</p>
<p>{{ basic.x8 }}</p>

<p>{{ basic.x9 }}</p>
<p>{{ basic.x10 }}</p>
<p>{{ basic.x11 }}</p>
<p>{{ basic.x12 }}</p>

<h1>const Basic</h1>
<p>{{ cbasic.a }}</p>
<p>{{ cbasic.b }}</p>
<!--<p>{{ cbasic.c }}</p> invalid for const object -->
<!--<p>{{ cbasic.d }}</p> invalid for const object -->
<!--<p>{{ cbasic.e }}</p> invalid for const object-->
<!--<p>{{ cbasic.f }}</p> invalid for const object-->
<p>{{ cbasic.x0 }}</p>
<p>{{ cbasic.x1 }}</p>
<p>{{ cbasic.x2 }}</p>
<!--<p>{{ cbasic.x3 }}</p> invalid for const object-->
<!--<p>{{ cbasic.x4 }}</p> invalid for const object-->
<p>{{ cbasic.x5 }}</p>
<p>{{ cbasic.x6 }}</p>
<p>{{ cbasic.x7 }}</p>
<p>{{ cbasic.x8 }}</p>
<!--<p>{{ cbasic.x9 }}</p> invalid for const object-->
<p>{{ cbasic.x10 }}</p>
<!--<p>{{ cbasic.x11 }}</p> invalid for const object-->
<p>{{ cbasic.x12 }}</p>

<h1>Wrapped</h1>
<p>{{ wrapped.a.val }}</p>
<p>{{ wrapped.b.val }}</p>
<p>{{ wrapped.c.val }}</p>
<p>{{ wrapped.d.val }}</p>
<p>{{ wrapped.e.val }}</p>
<p>{{ wrapped.x0.val }}</p>

<h1>Pointed</h1>
<p>{{ pointed.a.val }}</p>
<p>{{ pointed.b.val }}</p>
<p>{{ pointed.c.val }}</p>

<h1>Arrays</h1>
<p><span data-for="arrays.a">{{ it }} </span></p>
<p><span data-for="arrays.b">{{ it }} </span></p>
<p><span data-for="arrays.c">{{ it.val }} </span></p>
<p><span data-for="arrays.d">{{ it.val }} </span></p>
<p><span data-for="arrays.e">{{ it.val }} </span></p>

<p><span data-for="arrays.x0">{{ it }} </span></p>
<p><span data-for="arrays.x1">{{ it.val }} </span></p>
<p><span data-for="arrays.x2">{{ it }} </span></p>
<p><span data-for="arrays.x3">{{ it }} </span></p>
<p><span data-for="arrays.x4">{{ it.val }} </span></p>
<p><span data-for="arrays.x5">{{ it }} </span></p>
<p><span data-for="arrays.cx3">{{ it }} </span></p>
<p><span data-for="arrays.cx4">{{ it.val }} </span></p>
<p><span data-for="arrays.cx5">{{ it }} </span></p>

<p>The next line crashes the program:</p>
 <p><span data-for="arrays.x2">{{ it }} </span></p>

</div>
</body>
</rml>
)";

static const String inside_string_rml = R"(
<rml>
<head>
	<title>Test</title>
	<link type="text/template" href="/assets/window.rml"/>
	<style>
		body.window
		{
			left: 50px;
			right: 50px;
			top: 30px;
			bottom: 30px;
			max-width: -1px;
			max-height: -1px;
		}
	</style>
</head>

<body template="window">
<div data-model="basics">

<p>{{ i0 }}</p>
<p>{{ 'i0' }}</p>
<p>{{ 'i{}23' }}</p>
<p>before {{ 'i{{test}}23' }} test</p>
<p>a {{ 'i' }} b {{ 'j' }} c</p>
<p>{{i0}}</p>
<p>{{ 'i{}' }}</p>

</div>
</body>
</rml>
)";

static const String aliasing_rml = R"(
<rml>
<head>
	<title>Test</title>
	<link type="text/rcss" href="/assets/rml.rcss"/>
	<link type="text/rcss" href="/assets/invader.rcss"/>
	<link type="text/template" href="/../Tests/Data/UnitTests/data-title.rml"/>
	<style>
		body {
			width: 600px;
			height: 400px;
			background: #ccc;
			color: #333;
		}
		.title-wrapper { border: 1dp red; }
		.icon { width: 64dp; height: 64dp; display: inline-block; }
		.icon[icon="a"] { decorator: image("/assets/high_scores_alien_1.tga"); }
		.icon[icon="b"] { decorator: image("/assets/high_scores_alien_2.tga"); }
	</style>
</head>

<body data-model="basics">
<p>{{ i0 }}</p>
<p data-alias-differentname="i0">{{ differentname }}</p>
<div data-alias-title="s0" data-alias-icon="wrapped.a.val" id="w1">
	<template src="data-title"/>
</div>
<div data-alias-title="s1" data-alias-icon="wrapped.b.val" id="w2">
	<template src="data-title"/>
</div>
</body>
</rml>
)";

static const String dynamic_rml = R"(
<rml>
<head>
	<title>Test</title>
	<link type="text/rcss" href="/assets/rml.rcss"/>
	<link type="text/rcss" href="/assets/invader.rcss"/>
	<link type="text/template" href="/../Tests/Data/UnitTests/data-title.rml"/>
</head>

<body data-model="basics">
<p>{{ arrays.a[0] }}</p>
<p>{{ arrays.a[i0] }}</p>
<p>{{ arrays.b[i1] }}</p>
<p>{{ arrays.c[arrays.b[i1] - 19].val }}</p>
<p>{{ arrays.c[sqrt(arrays.b[i1] - 12) - 1].val }}</p>
</body>
</rml>
)";

// using RAII to make sure that resources get unloaded when unhandled exception occurs
struct Guard
{
	Guard(ElementDocument* e)
	{
		this->e = e;
	}
	~Guard()
	{
		if(e!= nullptr)
			e->Close();

		TestsShell::ShutdownShell();
	}

	ElementDocument *e;
};


struct StringWrap
{
	StringWrap(String val = "wrap_default") : val(val) {}
	String val;
};

enum SimpleEnum { Simple_Zero = 0, Simple_One, Simple_Two };

enum class ScopedEnum : uint64_t { Zero = 0, One, Two };

struct WrapperWrap
{
	StringWrap i0{"i0"};
	StringWrap* i1 = new StringWrap("i1");
	UniquePtr<StringWrap> i2 = MakeUnique<StringWrap>("i2");
	SharedPtr<StringWrap> i3 = MakeShared<StringWrap>("i3");

	const StringWrap x0{"x0"};
	const StringWrap* x1 = new StringWrap("x1");
	const UniquePtr<StringWrap> x2 = MakeUnique<StringWrap>("x2");
	const SharedPtr<StringWrap> x3 = MakeShared<StringWrap>("x3");
	const UniquePtr<const StringWrap> x4 = MakeUnique<const StringWrap>("x4");
	const SharedPtr<const StringWrap> x5 = MakeShared<const StringWrap>("x5");
};
WrapperWrap* g_wrapperwrap = nullptr;

struct Globals
{
	int i0 = 0;
	int* i1 = new int(1);
	UniquePtr<int> i2 = MakeUnique<int>(2);
	SharedPtr<int> i3 = MakeShared<int>(3);

	SimpleEnum simple = Simple_One;
	ScopedEnum scoped = ScopedEnum::One;

	String s0 = "s0";
	String* s1 = new String("s1");
	StringWrap s2 = StringWrap("s2");
	StringWrap* s3 = new StringWrap("s3");
	UniquePtr<StringWrap> s4 = MakeUnique<StringWrap>("s4");
	SharedPtr<StringWrap> s5 = MakeShared<StringWrap>("s5");

	const int x0 = 100;
	const int* x1 = new int(101);
	UniquePtr<const int> x2 = MakeUnique<int>(102);
	const StringWrap* x3 = new StringWrap("x2");
	UniquePtr<const StringWrap> x4 = MakeUnique<StringWrap>("x3");
} *globals;


struct Basic {
	int a = 1;
	int* b = new int(2);

	SimpleEnum simple = Simple_One;
	ScopedEnum scoped = ScopedEnum::One;

	int GetC()
	{
		static int v = 5;
		return v;
	}
	int& GetD()
	{
		static int v = 5;
		return v;
	}
	int* GetE()
	{
		static int v = 6;
		return &v;
	}
	UniquePtr<int> GetF() { return MakeUnique<int>(7); }

	const int x0 = 2;
	const int* x1 = new int(3);
	int GetX2() const { return 4; }
	const int& GetX3() {
		static int g = 7;
		return g;
	}
	const int* GetX4() {
		static int h = 8;
		return &h;
	}
	const int& GetX5() const {
		static int h = 9;
		return h;
	}
	const int* GetX6() const {
		static int i = 10;
		return &i;
	}
	int& GetX7() const {
		static int h = 11;
		return h;
	}
	int* GetX8() const {
		static int i = 12;
		return &i;
	}

	// const pointers
	const int* const GetX9() {
		static int h = 13;
		return &h;
	}
	const int* const GetX10() const {
		static int i = 14;
		return &i;
	}
	int* const GetX11()  {
		static int i = 15;
		return &i;
	}
	int* const GetX12() const {
		static int i = 16;
		return &i;
	}
	// Invalid: Illegal signature
	int GetX5(int) { return 9; }
};
Basic* g_basic;
const Basic* g_cbasic;

struct Wrapped {
	StringWrap a = {"a"};
	StringWrap* b = new StringWrap("b");
	UniquePtr<StringWrap> c = MakeUnique<StringWrap>("c");

	StringWrap& GetD()
	{
		static StringWrap v = {"e"};
		return v;
	}
	StringWrap* GetE()
	{
		static StringWrap v = {"f"};
		return &v;
	}

	const StringWrap* x0 = new StringWrap("x0");

	// Invalid (run-time): Returning non-scalar variable by value.
	StringWrap GetX1() { return {"x1"}; }
	// Invalid (run-time): Returning non-scalar variable by value.
	UniquePtr<StringWrap> GetX2() { return MakeUnique<StringWrap>("x2"); }
};
Wrapped* g_wrapped;

using StringWrapPtr = UniquePtr<StringWrap>;

struct Pointed {
	StringWrapPtr a = MakeUnique<StringWrap>("a");

	StringWrapPtr& GetB()
	{
		static StringWrapPtr v = MakeUnique<StringWrap>("b");
		return v;
	}
	StringWrapPtr* GetC()
	{
		static StringWrapPtr v = MakeUnique<StringWrap>("c");
		return &v;
	}

	// Invalid: We disallow recursive pointer types (pointer to pointer)
	StringWrapPtr* x0 = new StringWrapPtr(new StringWrap("x0"));

	// Invalid (run-time error): Only scalar data members can be returned by value
	StringWrapPtr GetX1() { return MakeUnique<StringWrap>("x1"); }
};
Pointed* g_pointed = nullptr;

struct Arrays {
	Vector<int> a = {10, 11, 12};
	Vector<int*> b = {new int(20), new int(21), new int(22)};
	Vector<StringWrap> c = {StringWrap("c1"), StringWrap("c2"), StringWrap("c3")};
	Vector<StringWrap*> d = {new StringWrap("d1"), new StringWrap("d2"), new StringWrap("d3")};
	Vector<StringWrapPtr> e;

	Vector<const int*> x0 = { new int(30), new int(31), new int(32) };
	Vector<UniquePtr<const StringWrap>> x1;
	const Vector<int*> x2 = { new int(41), new int(42), new int(43) };

	Vector<const int*>& getX3()
	{
		static auto p = Vector<const int*>{ new int(51), new int(52), new int(53) };
		return p;
	}
	Vector<UniquePtr<const StringWrap>>& getX4()
	{
		static Vector<UniquePtr<const StringWrap>>* p = nullptr;
		if(p == nullptr) {
			p = new Vector<UniquePtr<const StringWrap>>;
			p->emplace_back(MakeUnique<StringWrap>("x4_1"));
			p->emplace_back(MakeUnique<StringWrap>("x4_2"));
			p->emplace_back(MakeUnique<StringWrap>("x4_3"));
		}
		return *p;
	}
	const Vector<int*>& getX5()
	{
		static const auto p = Vector<int*>{ new int(61), new int(62), new int(63) };
		return p;
	}

	const Vector<const int*>& getcX3() const
	{
		static auto p = Vector<const int*>{ new int(71), new int(72), new int(73) };
		return p;
	}
	const Vector<UniquePtr<const StringWrap>>& getcX4() const
	{
		static Vector<UniquePtr<const StringWrap>>* p = nullptr;
		if(p == nullptr) {
			p = new Vector<UniquePtr<const StringWrap>>;
			p->emplace_back(MakeUnique<StringWrap>("cx4_1"));
			p->emplace_back(MakeUnique<StringWrap>("cx4_2"));
			p->emplace_back(MakeUnique<StringWrap>("cx4_3"));
		}
		return *p;
	}
	const Vector<int*>& getcX5() const
	{
		static const auto p = Vector<int*>{ new int(81), new int(82), new int(83) };
		return p;
	}

	Arrays() {
		e.emplace_back(MakeUnique<StringWrap>("e1"));
		e.emplace_back(MakeUnique<StringWrap>("e2"));
		e.emplace_back(MakeUnique<StringWrap>("e3"));
		x1.emplace_back(MakeUnique<StringWrap>("x1_1"));
		x1.emplace_back(MakeUnique<StringWrap>("x1_2"));
		x1.emplace_back(MakeUnique<StringWrap>("x1_3"));
	}

};
Arrays* g_arrays = nullptr;
const Arrays* g_carrays = nullptr;

DataModelHandle model_handle;

TEST_CASE("databinding.types")
{
	static_assert(!PointerTraits<int>::is_pointer::value, "");
	static_assert(!PointerTraits<int&>::is_pointer::value, "");
	static_assert(PointerTraits<int*>::is_pointer::value, "");
	static_assert(PointerTraits<UniquePtr<int>>::is_pointer::value, "");
	static_assert(PointerTraits<UniquePtr<const int>>::is_pointer::value, "");
	static_assert(PointerTraits<SharedPtr<int>>::is_pointer::value, "");
	static_assert(PointerTraits<SharedPtr<const int>>::is_pointer::value, "");

	static_assert(std::is_same<int,       PointerTraits<int>::element_type>::value, "");
	static_assert(std::is_same<const int, PointerTraits<const int>::element_type>::value, "");
	static_assert(std::is_same<int&,      PointerTraits<int&>::element_type>::value, "");

	static_assert(std::is_same<int,  PointerTraits<int*>::element_type>::value, "");
	static_assert(std::is_same<int,  PointerTraits<UniquePtr<int>>::element_type>::value, "");
	static_assert(std::is_same<int,  PointerTraits<SharedPtr<int>>::element_type>::value, "");
	static_assert(std::is_same<int,  PointerTraits<SharedPtr<int>>::element_type>::value, "");
	static_assert(std::is_same<int*, PointerTraits<int**>::element_type>::value, "");


	{
		int x = 10;
		DataPointer ptr(&x);
		*ptr.Get<int*>() += 5;

		CHECK(x == 15);
		CHECK(x == *ptr.Get<int*>());
	}

	{
		UniquePtr<int> u = MakeUnique<int>(20);
		DataPointer ptr(&u);
		CHECK(ptr.Get<UniquePtr<int>*>() == &u);

		DataPointer ptr_underlying = PointerTraits< UniquePtr<int> >::Dereference(ptr);
		CHECK(ptr_underlying.Get<int*>() == u.get());

		int u_out = *ptr_underlying.Get<int*>();
		CHECK(*u == u_out);

		*ptr_underlying.Get<int*>() += 5;
		CHECK(*u == 25);
	}

	{
		UniquePtr<const int> u = MakeUnique<const int>(20);
		DataPointer ptr(&u);
		CHECK(ptr.Get<UniquePtr<const int>*>() == &u);

		DataPointer ptr_underlying = PointerTraits< UniquePtr<const int> >::Dereference(ptr);
		CHECK(ptr_underlying.Get<const int*>() == u.get());
		CHECK(ptr_underlying.Get<int*>() == nullptr);

		int u_out = *ptr_underlying.Get<const int*>();
		CHECK(*u == u_out);
	}
}

bool InitializeDataBindings(Context* context)
{
	Rml::DataModelConstructor constructor = context->CreateDataModel("basics");
	if (!constructor)
		return false;

	constructor.RegisterTransformFunc("sqrt", [](const VariantList& params) {
		if (params.empty())
			return Variant();
		return Variant(std::sqrt(params[0].Get<int>()));
	});

	if (auto handle = constructor.RegisterStruct<StringWrap>())
	{
		handle.RegisterMember("val", &StringWrap::val);
	}

	if (auto handle = constructor.RegisterStruct<WrapperWrap>())
	{
		handle.RegisterMember("i0", &WrapperWrap::i0);
		handle.RegisterMember("i1", &WrapperWrap::i1);
		handle.RegisterMember("i2", &WrapperWrap::i2);
		handle.RegisterMember("i3", &WrapperWrap::i3);

		handle.RegisterMember("x0", &WrapperWrap::x0);
		handle.RegisterMember("x1", &WrapperWrap::x1);
		handle.RegisterMember("x2", &WrapperWrap::x2);
		handle.RegisterMember("x3", &WrapperWrap::x3);
		handle.RegisterMember("x4", &WrapperWrap::x4);
		handle.RegisterMember("x5", &WrapperWrap::x5);
	}
	g_wrapperwrap = new WrapperWrap;
	constructor.Bind("wrapperwrap", g_wrapperwrap);

	globals = new Globals();
	{
		// Globals
		constructor.Bind("i0", &globals->i0);
		constructor.Bind("i1", &globals->i1);
		constructor.Bind("i2", &globals->i2);
		constructor.Bind("i3", &globals->i3);

		constructor.Bind("s0", &globals->s0);
		constructor.Bind("s1", &globals->s1);
		constructor.Bind("s2", &globals->s2);
		constructor.Bind("s3", &globals->s3);
		constructor.Bind("s4", &globals->s4);
		constructor.Bind("s5", &globals->s5);

		constructor.Bind("simple", &globals->simple);
		constructor.Bind("scoped", &globals->scoped);
		constructor.Bind("x0", &globals->x0);
		constructor.Bind("x1", &globals->x1);
		constructor.Bind("x2", &globals->x2);
		constructor.Bind("x3", &globals->x3);
		constructor.Bind("x4", &globals->x4);
	}

	if (auto handle = constructor.RegisterStruct<Basic>())
	{
		handle.RegisterMember("a", &Basic::a);
		handle.RegisterMember("b", &Basic::b);
		handle.RegisterMember("c", &Basic::GetC);
		handle.RegisterMember("d", &Basic::GetD);
		handle.RegisterMember("e", &Basic::GetE);
		handle.RegisterMember("f", &Basic::GetF);
		handle.RegisterMember("simple", &Basic::simple);
		handle.RegisterMember("scoped", &Basic::scoped);

		handle.RegisterMember("x0", &Basic::x0);
		handle.RegisterMember("x1", &Basic::x1);
		handle.RegisterMember("x2", &Basic::GetX2);
		handle.RegisterMember("x3", &Basic::GetX3);
		handle.RegisterMember("x4", &Basic::GetX4);
		handle.RegisterMember("x5", &Basic::GetX5);
		handle.RegisterMember("x6", &Basic::GetX6);
		handle.RegisterMember("x7", &Basic::GetX7);
		handle.RegisterMember("x8", &Basic::GetX8);

		handle.RegisterMember("x9", &Basic::GetX9);
		handle.RegisterMember("x10", &Basic::GetX10);
		handle.RegisterMember("x11", &Basic::GetX11);
		handle.RegisterMember("x12", &Basic::GetX12);
	}
	g_basic = new Basic;
	constructor.Bind("basic", g_basic);
	g_cbasic = new Basic;
	constructor.Bind("cbasic", g_cbasic);

	if (auto handle = constructor.RegisterStruct<Wrapped>())
	{
		handle.RegisterMember("a", &Wrapped::a);
		handle.RegisterMember("b", &Wrapped::b);
		handle.RegisterMember("c", &Wrapped::c);
		handle.RegisterMember("d", &Wrapped::GetD);
		handle.RegisterMember("e", &Wrapped::GetE);

		handle.RegisterMember("x0", &Wrapped::x0);
		// handle.RegisterMember("x1", &Wrapped::GetX1);
		// handle.RegisterMember("x2", &Wrapped::GetX2);
	}
	g_wrapped = new Wrapped;
	constructor.Bind("wrapped", g_wrapped);

	if (auto handle = constructor.RegisterStruct<Pointed>())
	{
		handle.RegisterMember("a", &Pointed::a);
		handle.RegisterMember("b", &Pointed::GetB);
		handle.RegisterMember("c", &Pointed::GetC);

		// handle.RegisterMember("x0", &Pointed::x0);
		// handle.RegisterMember("x1", &Pointed::GetX1);
	}
	g_pointed = new Pointed;
	constructor.Bind("pointed", g_pointed);

	constructor.RegisterArray<decltype(Arrays::a)>();
	constructor.RegisterArray<decltype(Arrays::b)>();
	constructor.RegisterArray<decltype(Arrays::c)>();
	constructor.RegisterArray<decltype(Arrays::d)>();
	constructor.RegisterArray<decltype(Arrays::e)>();

	constructor.RegisterArray<decltype(Arrays::x0)>();
	constructor.RegisterArray<decltype(Arrays::x1)>();
    // constructor.RegisterArray<decltype(Arrays::x2)>();

	if (auto handle = constructor.RegisterStruct<Arrays>())
	{
		handle.RegisterMember("a", &Arrays::a);
		handle.RegisterMember("b", &Arrays::b);
		handle.RegisterMember("c", &Arrays::c);
		handle.RegisterMember("d", &Arrays::d);
		handle.RegisterMember("e", &Arrays::e);

		handle.RegisterMember("x0", &Arrays::x0);
		handle.RegisterMember("x1", &Arrays::x1);
		handle.RegisterMember("x2", &Arrays::x2);
		handle.RegisterMember("x3", &Arrays::getX3);
		handle.RegisterMember("x4", &Arrays::getX4);
		handle.RegisterMember("x5", &Arrays::getX5);
		handle.RegisterMember("cx3", &Arrays::getcX3);
		handle.RegisterMember("cx4", &Arrays::getcX4);
		handle.RegisterMember("cx5", &Arrays::getcX5);
	}
	g_arrays = new Arrays;
	constructor.Bind("arrays", g_arrays);
	g_carrays = new Arrays;
	constructor.Bind("carrays", g_carrays);

	model_handle = constructor.GetModelHandle();

	return true;
}

} // Anonymous namespace

TEST_CASE("data_binding")
{
	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	REQUIRE(InitializeDataBindings(context));

	ElementDocument* document = context->LoadDocumentFromMemory(document_rml);
	REQUIRE(document);
	document->Show();
	// auto g = Guard(document);

	TestsShell::RenderLoop();

	document->Close();

	TestsShell::ShutdownShell();
}

enum TestSettings {
	cOnlyReadable = 0b0,
	cWritable = 0b1,
	cPassedByValue = 0b10,
	cMemberNotAvailable = 0b100,
	cNoSetter = cWritable | cPassedByValue
};

template<typename T>
void test_variable(DataModel *model, const char *name, T getter, long flags) {
	using ResultType = typename std::result_of<T()>::type;
	using RT = typename std::remove_cv<typename std::remove_pointer<ResultType>::type>::type;
	Variant variant;
	auto addr = Rml::ParseAddress(name);

	if(flags & cMemberNotAvailable) // check if member is not available
	{
		CHECK_THROWS(model->GetVariableInto(addr, variant));
		return; // no need to continue testing if member doesn't exist
	}

	{
		auto res = model->GetVariableInto(addr, variant);
		CHECK(res == true);
		CHECK(variant.Get<RT>() == getter());
	}
	{
		auto new_value = ResultType();
		auto var = model->GetVariable(addr);
		if(flags & cWritable) // check if assignment succeeds
		{
			if(flags & cPassedByValue) // value only variables will result in failed assigment with 'soft' error
			{
				CHECK(!var.Set(Variant(new_value)));
			}
			else
			{
				CHECK(var.Set(Variant(new_value)));
			}
			auto res = model->GetVariableInto(addr, variant);
			CHECK(res == true);
			if(! (flags & cPassedByValue)) // value only variables won't change actual variable
				CHECK(variant.Get<RT>() == new_value);
		}
		else // check if assignment fails
		{
			CHECK_THROWS(var.Set(Variant(new_value)));
			auto res = model->GetVariableInto(addr, variant);
			CHECK(res == true);
			CHECK(variant.Get<RT>() != new_value);
		}
		CHECK(variant.Get<RT>() == getter());
	}
}

#define RML_TEST_VARIABLE_I(x, y, z) \
{                              \
    SUBCASE(#x) {                    \
        auto getter = []() { return y; };    \
        test_variable(model, #x, getter, z); \
    }                               \
}


TEST_CASE("data_binding.getset")
{
	rml_throw_on_errors = true;
	rml_stop_on_asserts = false;

	Context* context = TestsShell::GetContext();
	REQUIRE(context);
	auto g = Guard(nullptr);

	REQUIRE(InitializeDataBindings(context));

	TestsShell::RenderLoop();

	auto model = context->GetDataModel("basics").GetModelHandle().model;


	RML_TEST_VARIABLE_I(wrapperwrap.i0.val, g_wrapperwrap->i0.val, cWritable);
	RML_TEST_VARIABLE_I(wrapperwrap.i1.val, g_wrapperwrap->i1->val, cWritable);
	RML_TEST_VARIABLE_I(wrapperwrap.i2.val, g_wrapperwrap->i2->val, cWritable);
	RML_TEST_VARIABLE_I(wrapperwrap.i3.val, g_wrapperwrap->i3->val, cWritable);
	RML_TEST_VARIABLE_I(wrapperwrap.x0.val, g_wrapperwrap->x0.val, cOnlyReadable);
	RML_TEST_VARIABLE_I(wrapperwrap.x1.val, g_wrapperwrap->x1->val, cOnlyReadable);
	RML_TEST_VARIABLE_I(wrapperwrap.x2.val, g_wrapperwrap->x2->val, cWritable);
	RML_TEST_VARIABLE_I(wrapperwrap.x3.val, g_wrapperwrap->x3->val, cWritable);
	RML_TEST_VARIABLE_I(wrapperwrap.x4.val, g_wrapperwrap->x4->val, cOnlyReadable);
	RML_TEST_VARIABLE_I(wrapperwrap.x5.val, g_wrapperwrap->x5->val, cOnlyReadable);

	RML_TEST_VARIABLE_I(i0, globals->i0, cWritable);
	RML_TEST_VARIABLE_I(i1, *globals->i1, cWritable);
	RML_TEST_VARIABLE_I(i2, *globals->i2, cWritable);
	RML_TEST_VARIABLE_I(i3, *globals->i3, cWritable);
	RML_TEST_VARIABLE_I(s0, globals->s0, cWritable);
	RML_TEST_VARIABLE_I(s1, *globals->s1, cWritable);
	RML_TEST_VARIABLE_I(s2.val, globals->s2.val, cWritable);
	RML_TEST_VARIABLE_I(s3.val, globals->s3->val, cWritable);
	RML_TEST_VARIABLE_I(s4.val, globals->s4->val, cWritable);
	RML_TEST_VARIABLE_I(s5.val, globals->s5->val, cWritable);
	RML_TEST_VARIABLE_I(x0, globals->x0, cOnlyReadable);
	RML_TEST_VARIABLE_I(x1, *globals->x1, cOnlyReadable);
	RML_TEST_VARIABLE_I(x2, *globals->x2, cOnlyReadable);
	RML_TEST_VARIABLE_I(x3.val, globals->x3->val, cOnlyReadable);
	RML_TEST_VARIABLE_I(x4.val, globals->x4->val, cOnlyReadable);


	RML_TEST_VARIABLE_I(basic.a , g_basic->a , cWritable);
	RML_TEST_VARIABLE_I(basic.b , *g_basic->b , cWritable);
	RML_TEST_VARIABLE_I( basic.c ,  g_basic->GetC() , cNoSetter);
	RML_TEST_VARIABLE_I(basic.d , g_basic->GetD() , cWritable);
	RML_TEST_VARIABLE_I(basic.e , *g_basic->GetE() , cWritable);
	RML_TEST_VARIABLE_I( basic.f ,  *g_basic->GetF() , cNoSetter);
	RML_TEST_VARIABLE_I(basic.x0 , g_basic->x0 , cOnlyReadable);
	RML_TEST_VARIABLE_I(basic.x1 , *g_basic->x1 , cOnlyReadable);
	RML_TEST_VARIABLE_I( basic.x2 ,  g_basic->GetX2() , cNoSetter);
	RML_TEST_VARIABLE_I(basic.x3 , g_basic->GetX3() , cOnlyReadable);
	RML_TEST_VARIABLE_I(basic.x4 , *g_basic->GetX4() , cOnlyReadable);
	RML_TEST_VARIABLE_I(basic.x5 , g_basic->GetX5() , cOnlyReadable);
	RML_TEST_VARIABLE_I(basic.x6 , *g_basic->GetX6() , cOnlyReadable);
	RML_TEST_VARIABLE_I(basic.x7 , g_basic->GetX7() , cWritable);
	RML_TEST_VARIABLE_I(basic.x8 , *g_basic->GetX8() , cWritable);
	RML_TEST_VARIABLE_I(basic.x9 , *g_basic->GetX9() , cOnlyReadable);
	RML_TEST_VARIABLE_I(basic.x10 , *g_basic->GetX10() , cOnlyReadable);
	RML_TEST_VARIABLE_I(basic.x11 , *g_basic->GetX11() , cWritable);
	RML_TEST_VARIABLE_I(basic.x12 , *g_basic->GetX12() , cWritable);


	RML_TEST_VARIABLE_I(cbasic.a, g_cbasic->a, cOnlyReadable);
	RML_TEST_VARIABLE_I(cbasic.b, *g_cbasic->b, cWritable);
	RML_TEST_VARIABLE_I(cbasic.c, 0, cMemberNotAvailable);
	RML_TEST_VARIABLE_I(cbasic.d, 0, cMemberNotAvailable);
    RML_TEST_VARIABLE_I(cbasic.e, 0, cMemberNotAvailable);
    RML_TEST_VARIABLE_I(cbasic.f, 0, cMemberNotAvailable);
	RML_TEST_VARIABLE_I(cbasic.x0, g_cbasic->x0, cOnlyReadable);
	RML_TEST_VARIABLE_I(cbasic.x1, *g_cbasic->x1, cOnlyReadable);
	RML_TEST_VARIABLE_I(cbasic.x2, g_cbasic->GetX2(), cNoSetter);
	RML_TEST_VARIABLE_I(cbasic.x3, 0, cMemberNotAvailable);
    RML_TEST_VARIABLE_I(cbasic.x4, 0, cMemberNotAvailable);
    RML_TEST_VARIABLE_I(cbasic.x5, g_cbasic->GetX5(), cOnlyReadable);
	RML_TEST_VARIABLE_I(cbasic.x6, *g_cbasic->GetX6(), cOnlyReadable);
	RML_TEST_VARIABLE_I(cbasic.x7, g_cbasic->GetX7(), cWritable);
	RML_TEST_VARIABLE_I(cbasic.x8, *g_cbasic->GetX8(), cWritable);
	RML_TEST_VARIABLE_I(cbasic.x9, 0, cMemberNotAvailable);
	RML_TEST_VARIABLE_I(cbasic.x10, *g_cbasic->GetX10(), cOnlyReadable);
	RML_TEST_VARIABLE_I(cbasic.x11, 0, cMemberNotAvailable);
 	RML_TEST_VARIABLE_I(cbasic.x12, *g_cbasic->GetX12(), cWritable);

	RML_TEST_VARIABLE_I(wrapped.a.val, g_wrapped->a.val, cWritable);
	RML_TEST_VARIABLE_I(wrapped.b.val, g_wrapped->b->val, cWritable);
	RML_TEST_VARIABLE_I(wrapped.c.val, g_wrapped->c->val, cWritable);
	RML_TEST_VARIABLE_I(wrapped.d.val, g_wrapped->GetD().val, cWritable);
	RML_TEST_VARIABLE_I(wrapped.e.val, g_wrapped->GetE()->val, cWritable);
	RML_TEST_VARIABLE_I(wrapped.x0.val, g_wrapped->x0->val, cOnlyReadable);

	RML_TEST_VARIABLE_I(pointed.a.val, g_pointed->a->val, cWritable);
	RML_TEST_VARIABLE_I(pointed.b.val, g_pointed->GetB()->val, cWritable);
	RML_TEST_VARIABLE_I(pointed.c.val, (*g_pointed->GetC())->val, cWritable);


	RML_TEST_VARIABLE_I(arrays.a[0], g_arrays->a[0], cWritable);
	RML_TEST_VARIABLE_I(arrays.b[0], *g_arrays->b[0], cWritable);
	RML_TEST_VARIABLE_I(arrays.c[0].val, g_arrays->c[0].val, cWritable);
	RML_TEST_VARIABLE_I(arrays.d[0].val, g_arrays->d[0]->val, cWritable);
	RML_TEST_VARIABLE_I(arrays.e[0].val, g_arrays->e[0]->val, cWritable);
	RML_TEST_VARIABLE_I(arrays.x0[0], *g_arrays->x0[0], cOnlyReadable);
	RML_TEST_VARIABLE_I(arrays.x1[0].val, g_arrays->x1[0]->val, cOnlyReadable);
	RML_TEST_VARIABLE_I(arrays.x2[0], *g_arrays->x2[0], cWritable);
	RML_TEST_VARIABLE_I(arrays.x3[0], *g_arrays->getX3()[0], cOnlyReadable);
	RML_TEST_VARIABLE_I(arrays.x4[0].val, g_arrays->getX4()[0]->val, cOnlyReadable);
	RML_TEST_VARIABLE_I(arrays.x5[0], *g_arrays->getX5()[0], cWritable);
	RML_TEST_VARIABLE_I(arrays.cx3[0], *g_arrays->getcX3()[0], cOnlyReadable);
	RML_TEST_VARIABLE_I(arrays.cx4[0].val, g_arrays->getcX4()[0]->val, cOnlyReadable);
	RML_TEST_VARIABLE_I(arrays.cx5[0], *g_arrays->getcX5()[0], cWritable);


	RML_TEST_VARIABLE_I(carrays.a[0], g_carrays->a[0], cOnlyReadable);
	RML_TEST_VARIABLE_I(carrays.b[0], *g_carrays->b[0], cWritable);
	RML_TEST_VARIABLE_I(carrays.c[0].val, g_carrays->c[0].val, cOnlyReadable);
	RML_TEST_VARIABLE_I(carrays.d[0].val, g_carrays->d[0]->val, cWritable);
	RML_TEST_VARIABLE_I(carrays.e[0].val, g_carrays->e[0]->val, cWritable);
	RML_TEST_VARIABLE_I(carrays.x0[0], *g_carrays->x0[0], cOnlyReadable);
	RML_TEST_VARIABLE_I(carrays.x1[0].val, g_carrays->x1[0]->val, cOnlyReadable);
	RML_TEST_VARIABLE_I(carrays.x2[0], *g_carrays->x2[0], cWritable);
	RML_TEST_VARIABLE_I(carrays.x3[0], 0, cMemberNotAvailable);
	RML_TEST_VARIABLE_I(carrays.x4[0].val, 0, cMemberNotAvailable);
	RML_TEST_VARIABLE_I(carrays.x5[0], 0, cMemberNotAvailable);
	RML_TEST_VARIABLE_I(carrays.cx3[0], *g_carrays->getcX3()[0], cOnlyReadable);
	RML_TEST_VARIABLE_I(carrays.cx4[0].val, g_carrays->getcX4()[0]->val, cOnlyReadable);
	RML_TEST_VARIABLE_I(carrays.cx5[0], *g_carrays->getcX5()[0], cWritable);


	ElementDocument* document = context->LoadDocumentFromMemory(document_rml);
	REQUIRE(document);
	document->Show();
	TestsShell::RenderLoop();
	document->Close();

	TestsShell::ShutdownShell();

}

TEST_CASE("data_binding.inside_string")
{
	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	REQUIRE(InitializeDataBindings(context));

	ElementDocument* document = context->LoadDocumentFromMemory(inside_string_rml);
	REQUIRE(document);
	document->Show();

	TestsShell::RenderLoop();

	CHECK(document->QuerySelector("p:nth-child(4)")->GetInnerRML() == "before i{{test}}23 test");
	CHECK(document->QuerySelector("p:nth-child(5)")->GetInnerRML() == "a i b j c");

	document->Close();

	TestsShell::ShutdownShell();
}
TEST_CASE("data_binding.aliasing")
{
	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	REQUIRE(InitializeDataBindings(context));

	ElementDocument* document = context->LoadDocumentFromMemory(aliasing_rml);
	REQUIRE(document);
	document->Show();

	TestsShell::RenderLoop();

	CHECK(document->QuerySelector("p:nth-child(1)")->GetInnerRML() == document->QuerySelector("p:nth-child(2)")->GetInnerRML());
	CHECK(document->QuerySelector("#w1 .title")->GetInnerRML() == "s0");
	CHECK(document->QuerySelector("#w1 .icon")->GetAttribute("icon", String()) == "a");
	CHECK(document->QuerySelector("#w2 .title")->GetInnerRML() == "s1");
	CHECK(document->QuerySelector("#w2 .icon")->GetAttribute("icon", String()) == "b");

	document->Close();

	TestsShell::ShutdownShell();
}

TEST_CASE("data_binding.dynamic_variables")
{
	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	REQUIRE(InitializeDataBindings(context));

	ElementDocument* document = context->LoadDocumentFromMemory(dynamic_rml);
	REQUIRE(document);
	document->Show();

	TestsShell::RenderLoop();

	CHECK(document->QuerySelector("p:nth-child(1)")->GetInnerRML() == "10");
	CHECK(document->QuerySelector("p:nth-child(2)")->GetInnerRML() == "10");
	CHECK(document->QuerySelector("p:nth-child(3)")->GetInnerRML() == "21");
	CHECK(document->QuerySelector("p:nth-child(4)")->GetInnerRML() == "c3");
	CHECK(document->QuerySelector("p:nth-child(5)")->GetInnerRML() == "c3");

	*globals->i1 = 0;
	context->GetDataModel("basics").GetModelHandle().DirtyVariable("i1");
	TestsShell::RenderLoop();

	CHECK(document->QuerySelector("p:nth-child(3)")->GetInnerRML() == "20");
	CHECK(document->QuerySelector("p:nth-child(4)")->GetInnerRML() == "c2");

	document->Close();
	*globals->i1 = 1;

	TestsShell::ShutdownShell();
}

static const String set_enum_rml = R"(
<rml>
<head>
	<title>Test</title>
	<link type="text/template" href="/assets/window.rml"/>
	<style>
		body.window {
			width: 500px;
			height: 400px;
		}
	</style>
</head>
<body template="window">
<div data-model="basics">
<p id="simple" data-event-click="simple = 2">{{ simple }}</p>
</div>
</body>
</rml>
)";

TEST_CASE("data_binding.set_enum")
{
	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	globals->simple = Simple_One;

	REQUIRE(InitializeDataBindings(context));

	ElementDocument* document = context->LoadDocumentFromMemory(set_enum_rml);
	REQUIRE(document);
	document->Show();

	TestsShell::RenderLoop();

	Element* element = document->GetElementById("simple");
	CHECK(element->GetInnerRML() == "1");

	element->DispatchEvent(EventId::Click, Dictionary());
	TestsShell::RenderLoop();

	CHECK(globals->simple == Simple_Two);
	CHECK(element->GetInnerRML() == "2");

	document->Close();
	TestsShell::ShutdownShell();
}
