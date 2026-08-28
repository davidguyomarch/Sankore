/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 * Copyright (C) 2026 David Guyomarch
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */


#import <Carbon/Carbon.h>
#import <dlfcn.h>

/*
 * Returns an array of CFDictionaryRef types, each of which contains information about one of the processes.
 * The processes are ordered in front to back, i.e. in the same order they appear when typing command + tab, from left to right.
 * See the ProcessInformationCopyDictionary function documentation for the keys used in the dictionaries.
 * If something goes wrong, then this function returns nullptr.
 */
CFArrayRef CopyLaunchedApplicationsInFrontToBackOrder(void)
{    
    CFArrayRef (*_LSCopyApplicationArrayInFrontToBackOrder)(uint32_t sessionID) = nullptr;
    void       (*_LSASNExtractHighAndLowParts)(void const* asn, UInt32* psnHigh, UInt32* psnLow) = nullptr;
    CFTypeID   (*_LSASNGetTypeID)(void) = nullptr;
    
    void *lsHandle = dlopen("/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/LaunchServices", RTLD_LAZY);
    if (!lsHandle) { return nullptr; }
    
    _LSCopyApplicationArrayInFrontToBackOrder = (CFArrayRef(*)(uint32_t))dlsym(lsHandle, "_LSCopyApplicationArrayInFrontToBackOrder");
    _LSASNExtractHighAndLowParts = (void(*)(void const*, UInt32*, UInt32*))dlsym(lsHandle, "_LSASNExtractHighAndLowParts");
    _LSASNGetTypeID = (CFTypeID(*)(void))dlsym(lsHandle, "_LSASNGetTypeID");
    
    if (_LSCopyApplicationArrayInFrontToBackOrder == nullptr || _LSASNExtractHighAndLowParts == nullptr || _LSASNGetTypeID == nullptr) { return nullptr; }
    
    CFMutableArrayRef orderedApplications = CFArrayCreateMutable(kCFAllocatorDefault, 64, &kCFTypeArrayCallBacks);
    if (!orderedApplications) { return nullptr; }
    
    CFArrayRef apps = _LSCopyApplicationArrayInFrontToBackOrder(-1);
    if (!apps) { CFRelease(orderedApplications); return nullptr; }
    
    CFIndex count = CFArrayGetCount(apps);
    for (CFIndex i = 0; i < count; i++)
    {
        ProcessSerialNumber psn = {0, kNoProcess};
        CFTypeRef asn = CFArrayGetValueAtIndex(apps, i);
        if (CFGetTypeID(asn) == _LSASNGetTypeID())
        {
            _LSASNExtractHighAndLowParts(asn, &psn.highLongOfPSN, &psn.lowLongOfPSN);
            
            CFDictionaryRef processInfo = ProcessInformationCopyDictionary(&psn, kProcessDictionaryIncludeAllInformationMask);
            if (processInfo)
            {
                CFArrayAppendValue(orderedApplications, processInfo);
                CFRelease(processInfo);
            }
        }
    }
    CFRelease(apps);
    
    CFArrayRef result = CFArrayGetCount(orderedApplications) == 0 ? nullptr : CFArrayCreateCopy(kCFAllocatorDefault, orderedApplications);
    CFRelease(orderedApplications);
    return result;
}
