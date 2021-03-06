// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_SCRIPT_OTSCRIPTCHAI_HPP
#define OPENTXS_CORE_SCRIPT_OTSCRIPTCHAI_HPP

#include "opentxs/Forward.hpp"

#if OT_SCRIPT_CHAI
#include "opentxs/core/script/OTScript.hpp"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702)  // warning C4702: unreachable code
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

// SUBCLASS:  CHAI SCRIPT

namespace chaiscript
{
class ChaiScript;
}

namespace opentxs
{

class OTScriptChai : public OTScript
{
public:
    OTScriptChai();
    OTScriptChai(const String& strValue);
    OTScriptChai(const char* new_string);
    OTScriptChai(const char* new_string, size_t sizeLength);
    OTScriptChai(const std::string& new_string);

    virtual ~OTScriptChai();

    bool ExecuteScript(OTVariable* pReturnVar = nullptr) override;
    chaiscript::ChaiScript* const chai_{nullptr};
};
}  // namespace opentxs
#endif  // OT_SCRIPT_CHAI
#endif
