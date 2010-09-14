/**********************************************************\ 
Original Author: Richard Bateman (taxilian)

Created:    Oct 16, 2009
License:    Dual license model; choose one of two:
            New BSD License
            http://www.opensource.org/licenses/bsd-license.php
            - or -
            GNU Lesser General Public License, version 2.1
            http://www.gnu.org/licenses/lgpl-2.1.html

Copyright 2009 Richard Bateman, Firebreath development team
\**********************************************************/

#include "NPObjectAPI.h"
#include "NpapiBrowserHost.h"

using namespace FB::Npapi;

NPObjectAPI::NPObjectAPI(NPObject *o, NpapiBrowserHostPtr h)
    : BrowserObjectAPI(h), browser(h), obj(o)
{
    if (browser && o != NULL) {
        browser->RetainObject(obj);
    }
}

NPObjectAPI::~NPObjectAPI(void)
{
    if (browser && obj != NULL) {
        browser->ReleaseObject(obj);
    }
}

void NPObjectAPI::getMemberNames(std::vector<std::string> &nameVector)
{
    NPIdentifier *idArray(NULL);
    uint32_t count;

    browser->Enumerate(obj, &idArray, &count);
    for (uint32_t i = 0; i < count; i++) {
        nameVector.push_back(browser->StringFromIdentifier(idArray[i]));
    }
    browser->MemFree(idArray);
}

size_t NPObjectAPI::getMemberCount()
{
    NPIdentifier *idArray(NULL);
    uint32_t count;
    browser->Enumerate(obj, &idArray, &count);
    browser->MemFree(idArray);
    return (size_t)count;
}

bool NPObjectAPI::HasMethod(const std::string& methodName)
{
    return browser->HasMethod(obj, browser->GetStringIdentifier(methodName.c_str()));
}

bool NPObjectAPI::HasProperty(const std::string& propertyName)
{
    return browser->HasProperty(obj, browser->GetStringIdentifier(propertyName.c_str()));
}

bool NPObjectAPI::HasProperty(int idx)
{
    return browser->HasProperty(obj, browser->GetIntIdentifier(idx));
}

bool NPObjectAPI::HasEvent(const std::string& eventName)
{
    return false;
}


// Methods to manage properties on the API
FB::variant NPObjectAPI::GetProperty(const std::string& propertyName)
{
    NPVariant retVal;
    if (!browser->GetProperty(obj, browser->GetStringIdentifier(propertyName.c_str()), &retVal)) {
        throw script_error(propertyName.c_str());
    } else {
        FB::variant ret = browser->getVariant(&retVal);
        browser->ReleaseVariantValue(&retVal);
        return ret;
    }
}

void NPObjectAPI::SetProperty(const std::string& propertyName, const FB::variant& value)
{
    NPVariant val;
    browser->getNPVariant(&val, value);
    if (!browser->SetProperty(obj, browser->GetStringIdentifier(propertyName.c_str()), &val)) {
        throw script_error(propertyName.c_str());
    }
}

FB::variant NPObjectAPI::GetProperty(int idx)
{
    std::string strIdx(FB::variant(idx).convert_cast<std::string>());
    return GetProperty(strIdx);
}

void NPObjectAPI::SetProperty(int idx, const FB::variant& value)
{
    std::string strIdx(FB::variant(idx).convert_cast<std::string>());
    SetProperty(strIdx, value);
}

// Methods to manage methods on the API
FB::variant NPObjectAPI::Invoke(const std::string& methodName, const std::vector<FB::variant>& args)
{
    NPVariant retVal;

    if (!host->isMainThread()) {
        return InvokeMainThread(methodName, args);
    }

    // Convert the arguments to NPVariants
    NPVariant *npargs = new NPVariant[args.size()];
    for (unsigned int i = 0; i < args.size(); i++) {
        browser->getNPVariant(&npargs[i], args[i]);
    }

    bool res = false;
    // Invoke the method ("" means invoke default method)
    if (methodName.size() > 0) {
        res = browser->Invoke(obj, browser->GetStringIdentifier(methodName.c_str()), npargs, args.size(), &retVal);
    } else {
        res = browser->InvokeDefault(obj, npargs, args.size(), &retVal);
    }

    // Free the NPVariants that we earlier allocated
    for (unsigned int i = 0; i < args.size(); i++) {
        browser->ReleaseVariantValue(&npargs[i]);
    }

    if (!res) { // If the method call failed, throw an exception
        throw script_error(methodName.c_str());
    } else {
        FB::variant ret = browser->getVariant(&retVal);
        browser->ReleaseVariantValue(&retVal);  // Always release the return value!
        return ret;
    }
}
