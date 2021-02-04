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

#include "../../Include/RmlUi/Core/DataVariable.h"

namespace Rml {

bool DataVariable::Get(Variant& variant) {
    return definition->Get(ptr, variant);
}

bool DataVariable::Set(const Variant& variant) {
    return definition->Set(ptr, variant);
}

int DataVariable::Size() {
    return definition->Size(ptr);
}

DataVariable DataVariable::Child(const DataAddressEntry& address) {
    return definition->Child(ptr, address);
}

DataVariableType DataVariable::Type() {
    return definition->Type();
}


bool VariableDefinition::Get(VariablePointer /*ptr*/, Variant& /*variant*/) {
    Log::Message(Log::LT_WARNING, "Values can only be retrieved from scalar data types.");
    return false;
}
bool VariableDefinition::Set(VariablePointer /*ptr*/, const Variant& /*variant*/) {
    Log::Message(Log::LT_WARNING, "Values can only be assigned to scalar data types.");
    return false;
}
int VariableDefinition::Size(VariablePointer /*ptr*/) {
    Log::Message(Log::LT_WARNING, "Tried to get the size from a non-array data type.");
    return 0;
}
DataVariable VariableDefinition::Child(VariablePointer /*ptr*/, const DataAddressEntry& /*address*/) {
    Log::Message(Log::LT_WARNING, "Tried to get the child of a scalar type.");
    return DataVariable();
}

class LiteralIntDefinition final : public VariableDefinition {
public:
    LiteralIntDefinition() : VariableDefinition(DataVariableType::Scalar) {}

    bool Get(VariablePointer ptr, Variant& variant) override
    {
        variant = static_cast<int>(reinterpret_cast<intptr_t>(ptr.ptr));
        return true;
    }
};

DataVariable MakeLiteralIntVariable(int value)
{
    static LiteralIntDefinition literal_int_definition;
    return DataVariable(&literal_int_definition, reinterpret_cast<void*>(static_cast<intptr_t>(value)));
}

bool BasePointerDefinition::Get(VariablePointer ptr, Variant& variant) {
    return underlying_definition->Get(DereferencePointer(ptr), variant);
}

bool BasePointerDefinition::Set(VariablePointer ptr, const Variant& variant) {
    return underlying_definition->Set(DereferencePointer(ptr), variant);
}

int BasePointerDefinition::Size(VariablePointer ptr) {
    return underlying_definition->Size(DereferencePointer(ptr));
}

DataVariable BasePointerDefinition::Child(VariablePointer ptr, const DataAddressEntry& address) {
    return underlying_definition->Child(DereferencePointer(ptr), address);
}

} // namespace Rml
