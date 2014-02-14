#ifndef APPINFO_H
#define APPINFO_H

#define stringify(v1) #v1
#define quote(v1)  stringify(v1)

#define V_MAJOR 0
#define V_MINOR 9
#define V_BUILD 5
#define V_REVISION 0
#define V_ALPHA 0
#define V_RC 0

#define V_VERSION  V_MAJOR.V_MINOR.V_BUILD.V_REVISION
#define V_SVERSION  V_MAJOR.V_MINOR.V_BUILD

#if V_ALPHA>0
#define V_SVERSION_STR quote(V_SVERSION)"-a"
#elif V_RC>0
#define V_SVERSION_STR quote(V_SVERSION)"-rc"quote(V_RC)
#else
#define V_SVERSION_STR quote(V_SVERSION)
#endif

#define VER_FILEVERSION             V_MAJOR,V_MINOR,V_BUILD,V_REVISION
#define VER_FILEVERSION_STR         quote(V_VERSION)

#define VER_PRODUCTVERSION          V_MAJOR,V_MINOR,V_BUILD,V_REVISION
#define VER_PRODUCTVERSION_STR      quote(V_SVERSION)

#define VER_COMPANYNAME_STR         "Yury Khamenkov"
#define VER_FILEDESCRIPTION_STR     "qNotesManager"
#define VER_INTERNALNAME_STR        "qnotesmanager"
#define VER_ORIGINALFILENAME_STR    "qnotesmanager.exe"
#define VER_PRODUCTNAME_STR         "qNotesManager"
#define VER_COMMENTS                "qNotesManager is a program that will help you store and manage your notes"

#endif // APPINFO_H
