#include <ps4.h>

typedef struct _LncAppParam
{
  u32 size;
  u32 user_id;
  u32 app_option;
  u64 crash_report;
  u32 check_flag;
} LncAppParam;

int (*launchAppByTitleId)(const char *titleId, const char *argv[], LncAppParam *param);

int _main(void)
{
  initKernel();
  initLibc();
  initSysUtil();
  jailbreak();

  int prx = sceKernelLoadStartModule("/system/vsh/app/DMON00000/assets/libLncUtil.prx", 0, 0, 0, 0, 0);
  RESOLVE(prx, launchAppByTitleId);

  const char *argv[] = {NULL};

  LncAppParam param;
  memset(&param, 0, sizeof(param));
  param.size = sizeof(LncAppParam);
  param.user_id = -1;
  param.app_option = 0;
  param.crash_report = 0;
  param.check_flag = 0;
  return launchAppByTitleId("DMON00000", argv, &param);
}