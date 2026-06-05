#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>
#include <orbis/SystemService.h>

#include <libjbc.h>

#include <csignal>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <unistd.h>
#include <string>
#include <cstdarg>

#define TITLE_ID "DMON00000"
#define MNT_UPDATE 0x0000000000010000ULL

#define SYSCALL(nr, fn) \
  __attribute__((naked)) fn { asm volatile("mov $" #nr ", %rax\nmov %rcx, %r10\nsyscall\nret"); }

SYSCALL(378, static int nmount(struct iovec *iov, unsigned int niov, int flags))

extern unsigned char _binary_eboot_bin_start[], _binary_eboot_bin_end[];

static const std::string ROOT = "/system/vsh/app/DMON00000";
static const std::string SCE_SYS = ROOT + "/sce_sys";
static const std::string ASSETS = ROOT + "/assets";
static const std::string EBOOT = ROOT + "/eboot.bin";

static const std::string SRC_SFO =
    "/mnt/sandbox/NPXS21007_000/app0/sce_sys/param.sfo";
static const std::string DST_SFO = SCE_SYS + "/param.sfo";

static const std::string SRC_LNC_PRX =
    "/mnt/sandbox/DMON00001_000/app0/sce_module/libLncUtil.prx";
static const std::string DST_LNC_PRX = ASSETS + "/libLncUtil.prx";

bool (*appLaunchedByTitleId)(const char *titleId);

int (*killAppByTitleId)(const char *titleId);

int (*launchAppByTitleId)(const char *titleId,
                          const char *argv[],
                          LncAppParam *param);

static void build_iovec(struct iovec **iov, int *iovlen, const char *name, const void *val, size_t len)
{
  int i = *iovlen;
  *iov = (struct iovec *)realloc(*iov, sizeof(struct iovec) * (i + 2));
  if (!*iov)
  {
    *iovlen = -1;
    return;
  }

  (*iov)[i].iov_base = strdup(name);
  (*iov)[i].iov_len = strlen(name) + 1;

  ++i;
  (*iov)[i].iov_base = (void *)val;
  if (len == (size_t)-1)
    len = val ? strlen((const char *)val) + 1 : 0;
  (*iov)[i].iov_len = (int)len;

  *iovlen = i + 1;
}

static int mountfs(const char *device, const char *mountpoint, const char *fstype, const char *mode, uint64_t flags)
{
  struct iovec *iov = nullptr;
  int iovlen = 0;

  build_iovec(&iov, &iovlen, "fstype", fstype, -1);
  build_iovec(&iov, &iovlen, "fspath", mountpoint, -1);
  build_iovec(&iov, &iovlen, "from", device, -1);
  build_iovec(&iov, &iovlen, "large", "yes", -1);
  build_iovec(&iov, &iovlen, "timezone", "static", -1);
  build_iovec(&iov, &iovlen, "async", "", -1);
  build_iovec(&iov, &iovlen, "ignoreacl", "", -1);

  if (mode)
  {
    build_iovec(&iov, &iovlen, "dirmask", mode, -1);
    build_iovec(&iov, &iovlen, "mask", mode, -1);
  }

  int ret = nmount(iov, iovlen, flags);
  free(iov);

  return ret;
}

static bool file_exists(const std::string &path)
{
  struct stat buffer;
  return (stat(path.c_str(), &buffer) == 0);
}

static void copy_file(const std::string &source, const std::string &dest)
{
  int src = sceKernelOpen(source.c_str(), 0x0000, 0);
  if (src <= 0)
    return;

  int out = sceKernelOpen(dest.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  if (out <= 0)
  {
    sceKernelClose(src);
    return;
  }

  char *buffer = (char *)malloc(65536);
  if (buffer)
  {
    size_t bytes;
    while (0 < (bytes = sceKernelRead(src, buffer, 65536)))
      sceKernelWrite(out, buffer, bytes);
    free(buffer);
  }

  sceKernelClose(out);
  sceKernelClose(src);
}

static void extract_embed(const std::string &dst)
{
  int fd = sceKernelOpen(dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0777);
  if (fd <= 0)
    return;

  size_t size = (size_t)(_binary_eboot_bin_end - _binary_eboot_bin_start);
  if (size)
    sceKernelWrite(fd, _binary_eboot_bin_start, size);

  sceKernelClose(fd);
}

static jbc_jail_state backup;

static void boot_daemon_services()
{
  int prx = sceKernelLoadStartModule(SRC_LNC_PRX.c_str(),
                                     0, 0, 0, 0, 0);

  sceKernelDlsym(prx, "appLaunchedByTitleId",
                 (void **)&appLaunchedByTitleId);

  if (appLaunchedByTitleId(TITLE_ID))
  {
    sceKernelDlsym(prx, "killAppByTitleId",
                   (void **)&killAppByTitleId);

    killAppByTitleId(TITLE_ID);
  }

  sceKernelDlsym(prx, "launchAppByTitleId",
                 (void **)&launchAppByTitleId);

  const char *argv[] = {nullptr};

  LncAppParam param;
  memset(&param, 0, sizeof(param));
  param.size = sizeof(LncAppParam);
  param.user_id = -1;
  param.app_opt = 0;
  param.crash_report = 0;
  param.LaunchAppCheck_flag = LaunchApp_None;
  launchAppByTitleId(TITLE_ID, argv, &param);

  jbc_unjailbreak(&backup);
}

static void prepare_daemon_files()
{
  mountfs("/dev/da0x4.crypt", "/system", "exfatfs", "511", MNT_UPDATE);

  for (auto &p : {ROOT, SCE_SYS, ASSETS})
    mkdir(p.c_str(), 0777);

  extract_embed(EBOOT);
  copy_file(SRC_SFO, DST_SFO);
  copy_file(SRC_LNC_PRX, DST_LNC_PRX);

  boot_daemon_services();
}

int main()
{
  setvbuf(stdout, NULL, _IONBF, 0);

  jbc_jailbreak(&backup);

  sceSysmoduleLoadModuleInternal(static_cast<OrbisSysModuleInternal>(0x80000026));
  sceKernelLoadStartModule("/system/common/lib/libSceSysUtil.sprx", 0, 0, 0, 0, 0);

  sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_SYSTEM_SERVICE);
  sceKernelLoadStartModule("/system/common/lib/libSceSystemService.sprx", 0, 0, 0, 0, 0);

  sceSystemServiceHideSplashScreen();

  prepare_daemon_files();

  raise(SIGKILL);

  return 0;
}
