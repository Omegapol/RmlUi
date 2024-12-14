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

#include "CaptureScreen.h"
#include "TestConfig.h"
#include "TestNavigator.h"
#include "TestSuite.h"
#include "TestViewer.h"
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Debugger.h>
#include <PlatformExtensions.h>
#include <RmlUi_Backend.h>
#include <Shell.h>
#include <stdio.h>
#include <RmlUi/Core/DataModelHandle.h>
#include <../Source/Core/Elements/ElementGraph/DataGraphTypes.h>
#include <cmath>

Rml::Context* context = nullptr;
TestNavigator* g_navigator = nullptr;

double global_time = 0.0f;
double acc_time = 0.0f;

Rml::Vector<Rml::Vector2f> updated_vec;

void GameLoop()
{
	auto delta = Rml::GetSystemInterface()->GetElapsedTime() - global_time;
	global_time = Rml::GetSystemInterface()->GetElapsedTime();
	acc_time += delta;
	while(acc_time > 0.5) {
		updated_vec.push_back({(float) global_time, (float) global_time / 2.f});
		acc_time -= 0.5;
		auto model = Rml::GetContext(0)->GetDataModel("data").GetModelHandle();
		model.DirtyVariable("updated_vec");
	}

	context->Update();

	// auto shell_renderer = Rml::GetRenderInterface();
	// shell_renderer->PrepareRenderBuffer();
	context->Render();
	// shell_renderer->PresentRenderBuffer();

	if (g_navigator)
	{
		g_navigator->Update();
	}
}


#if defined RMLUI_PLATFORM_WIN32
	#include <RmlUi_Include_Windows.h>
	#include <RmlUi/Core/ElementDocument.h>
	#include <../../RmlUi/Source/Core/DataModel.h>
	#include <../../RmlUi/SOurce/Core/Elements/ElementGraph/DataFeed.h>
int APIENTRY WinMain(HINSTANCE /*instance_handle*/, HINSTANCE /*previous_instance_handle*/, char* win_command_line, int /*command_show*/)
#else
int main(int argc, char** argv)
#endif
{
	const char* command_line = nullptr;

#ifdef RMLUI_PLATFORM_WIN32
	command_line = win_command_line;
#else
	if (argc > 1)
		command_line = argv[1];
#endif
	int load_suite_index = -1;
	int load_case_index = -1;

	// Parse command line as <case_index> *or* <suite_index>:<case_index>.
	if (command_line)
	{
		int first_argument = -1;
		int second_argument = -1;
		const int num_arguments = sscanf(command_line, "%d:%d", &first_argument, &second_argument);

		switch (num_arguments)
		{
		case 1: load_case_index = first_argument - 1; break;
		case 2:
			load_suite_index = first_argument - 1;
			load_case_index = second_argument - 1;
			break;
		}
	}

	int window_width = 1500;
	int window_height = 800;

	// Initializes the shell which provides common functionality used by the included samples.
	if (!Shell::Initialize())
		return -1;

	// Constructs the system and render interfaces, creates a window, and attaches the renderer.
	if (!Backend::Initialize("Visual tests", window_width, window_height, true))
	{
		Shell::Shutdown();
		return -1;
	}

	// Install the custom interfaces constructed by the backend before initializing RmlUi.
	Rml::SetSystemInterface(Backend::GetSystemInterface());
	Rml::SetRenderInterface(Backend::GetRenderInterface());

	// RmlUi initialisation.
	Rml::Initialise();

	// Create the main RmlUi context.
	Rml::Context* context = Rml::CreateContext("main", Rml::Vector2i(window_width, window_height));
	if (!context)
	{
		Rml::Shutdown();
		Shell::Shutdown();
		return -1;
	}

	Rml::Debugger::Initialise(context);
	Shell::LoadFonts();
	context->SetDefaultScrollBehavior(Rml::ScrollBehavior::Instant, 1.f);

	Rml::DataModelConstructor constructor = context->CreateDataModel("data");
	if (auto handle = constructor.RegisterStruct<Rml::Vector2f>())
	{
		handle.RegisterMember("x", &Rml::Vector2f::x);
		handle.RegisterMember("y", &Rml::Vector2f::y);
	}
	constructor.RegisterArray<Rml::Vector<Rml::Vector2f>>();

	if (auto handle = constructor.RegisterStruct<Rml::CandleStickData>())
	{
		handle.RegisterMember("time", &Rml::CandleStickData::time);
		handle.RegisterMember("h", &Rml::CandleStickData::high);
		handle.RegisterMember("l", &Rml::CandleStickData::low);
		handle.RegisterMember("en", &Rml::CandleStickData::entry);
		handle.RegisterMember("ex", &Rml::CandleStickData::exit);
	}
	constructor.RegisterArray<Rml::Vector<Rml::CandleStickData>>();

	static auto vec = new Rml::Vector<Rml::Vector2f>{
			{10, 10},
			{20, 12},
			{30, 15},
			{40, 14},
			{50, 12},
			{60, 20},
			{70, 31},
			{80, 35},
			{90, 37},
			{100, 70},
			{110, 95},
			{120, 100},
			{150, -20},
			{170, -50},
			{210, 0},
			{240, 50}
	};
	constructor.Bind("vec", vec);

	static auto vec2 = new Rml::Vector<Rml::Vector2f>{
			{20, 50},
			{30, 55},
			{40, 57},
			{50, 100},
			{60, 58},
			{70, 52},
			{80, 60},
			{90, 50},
			{100, 30}
	};
	constructor.Bind("vec2", vec2);

	static auto candle = new Rml::Vector<Rml::CandleStickData>{
			{50, 30, 35, 40, 10},
			{53, 40, 40, 45, 15},
			{50, 30, 45, 50, 20},
			{60, 45, 50, 60, 25},
			{62, 38, 60, 40, 30},
			{45, 20, 40, 30, 35},
			{45, 26, 30, 35, 40}
	};
	constructor.Bind("candlevec", candle);

	static auto vec3 = new Rml::Vector<Rml::Vector2f>{};
	for(auto i = 0; i< 10000; ++i)
	{
		vec3->push_back({(float)i*5, (i%5)*10.f + (i /100) * 10});
	}
	constructor.Bind("vec3", vec3);

	static auto vec3h = new Rml::Vector<Rml::Vector2f>{};
	for(auto i = 0; i< 10000; ++i)
	{
		vec3h->push_back({(float)i/3.f, (i%5)*10.f + (i /100) * 10});
	}
	constructor.Bind("vec3h", vec3h);

	static auto vec3em = new Rml::Vector<Rml::Vector2f>{};
	constructor.Bind("vec_empty", vec3em);

	constructor.Bind("updated_vec", &updated_vec);

	static auto vec4 = new Rml::Vector<Rml::Vector2f>{};
	for(auto i = 0; i< 10000; ++i)
	{
		auto res = std::sinf((float) i / 5.f) + i/35.f;
		vec4->push_back({(float)i, res*15.f});
	}
	constructor.Bind("sine", vec4);

	{
		const Rml::StringList directories = GetTestInputDirectories();

		TestSuiteList test_suites;

		for (const Rml::String& directory : directories)
		{
			const Rml::StringList files = PlatformExtensions::ListFiles(directory, "rml");

			if (files.empty())
				Rml::Log::Message(Rml::Log::LT_WARNING, "Could not find any *.rml files in directory '%s'. Ignoring.", directory.c_str());
			else
				test_suites.emplace_back(directory, std::move(files));
		}

		RMLUI_ASSERTMSG(!test_suites.empty(), "RML test files directory not found or empty.");

		TestViewer viewer(context);

		TestNavigator navigator(Backend::GetRenderInterface(), context, &viewer, std::move(test_suites), load_suite_index, load_case_index);

		bool running = true;
		while (running)
		{
			auto delta = Rml::GetSystemInterface()->GetElapsedTime() - global_time;
			global_time = Rml::GetSystemInterface()->GetElapsedTime();
			acc_time += delta;
			while(acc_time > 0.5) {
				updated_vec.push_back({(float) global_time, (float) global_time / 2.f});
				acc_time -= 0.5;
				auto model = Rml::GetContext(0)->GetDataModel("data").GetModelHandle();
				model.DirtyVariable("updated_vec");
			}

			running = Backend::ProcessEvents(context, &Shell::ProcessKeyDownShortcuts);

			context->Update();

			Backend::BeginFrame();
			context->Render();
			navigator.Render();
			Backend::PresentFrame();

			navigator.Update();
		}
	}

	Rml::Shutdown();
	Shell::Shutdown();
	Backend::Shutdown();

	return 0;
}
