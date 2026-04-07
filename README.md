
# MyOS - A Minimal x86-64 Operating System

A educational operating system kernel written in C and Assembly for x86-64 architecture. This OS demonstrates core operating system concepts including memory management, process scheduling, interrupt handling, and system calls.

## Features

- **Bootloader Support**: Multiboot2 compliant (works with GRUB)
- **Memory Management**:
  - Physical Memory Manager (bitmap-based)
  - Virtual Memory Manager (paging with 4KB pages)
  - Dynamic kernel heap allocator (malloc/free)
- **Process Management**:
  - Round-robin process scheduler
  - Context switching
  - Process states (ready, running, waiting, zombie)
- **Interrupt Handling**:
  - Complete IDT setup
  - Exception handlers
  - IRQ handling (timer, keyboard)
- **System Calls**: Basic syscall interface (write, read, getpid, exit)
- **Shell**: Interactive command-line interface
- **Drivers**:
  - VGA text mode console (80x25 with color)
  - Serial port (COM1) for debugging
  - PS/2 keyboard input
- **Standard Library**: Custom printf, sprintf, string functions

## Project Structure

```
myos/
├── boot/
│   ├── grub/
│   │   └── grub.cfg           # GRUB bootloader configuration
│   ├── linker.ld              # Linker script
│   └── multiboot2.h           # Multiboot2 header definitions
├── src/
│   ├── kernel/
│   │   ├── main.c             # Kernel entry point
│   │   ├── boot.S             # 32-bit boot assembly
│   │   ├── gdt.c/h            # Global Descriptor Table
│   │   ├── idt.c/h            # Interrupt Descriptor Table
│   │   ├── paging.c/h         # Memory management
│   │   ├── heap.c/h           # Dynamic memory allocator
│   │   ├── scheduler.c/h      # Process scheduler
│   │   ├── syscall.c/h        # System call interface
│   │   ├── shell.c/h          # Command shell
│   │   ├── serial.c/h         # Serial port driver
│   │   └── gdt_flush.S        # GDT loading routine
│   └── lib/
│       ├── string.c/h         # String manipulation
│       ├── stdio.c/h          # Formatted I/O
│       └── stdarg.h           # Variable arguments
├── Makefile                   # Build system
├── run.sh                     # Build and run script
└── README.md                  # This file
```

## Prerequisites

### Build Requirements
- **GCC** (with x86_64 support)
- **GNU Make**
- **GRUB** (grub-mkrescue)
- **Xorriso** (for ISO creation)
- **QEMU** (for emulation)

### Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y gcc make grub-pc-bin grub-common xorriso qemu-system-x86
```

**Arch Linux:**
```bash
sudo pacman -S gcc make grub xorriso qemu
```

**Fedora/RHEL:**
```bash
sudo dnf install gcc make grub2 xorriso qemu-system-x86
```

## Building

### Quick Build
```bash
make all
```

### Build with Specific Target
```bash
# Build kernel binary only
make myos.bin

# Build bootable ISO
make myos.iso
```

### Clean Build
```bash
make clean
```

## Running

### Using QEMU (Recommended)
```bash
make run
```

### Manual QEMU Command
```bash
qemu-system-x86_64 -cdrom myos.iso -serial stdio -m 128M
```

### Running on Real Hardware
1. Write the ISO to a USB drive:
   ```bash
   sudo dd if=myos.iso of=/dev/sdX bs=4M status=progress
   ```
2. Boot from the USB drive (ensure BIOS is set to legacy boot mode)

## Shell Commands

Once the OS boots, you'll see the shell prompt: `myos>`

Available commands:

| Command | Description |
|---------|-------------|
| `help`  | Display available commands |
| `hello` | Print a greeting message |
| `mem`   | Show free memory information |
| `ps`    | List running processes |
| `clear` | Clear the screen |
| `reboot`| Reboot the system |

## Architecture Overview

### Boot Process
1. GRUB loads the kernel at 1MB mark (Multiboot2)
2. Boot.S sets up a temporary stack in 32-bit protected mode
3. Kernel initializes subsystems in order:
   - GDT (Global Descriptor Table)
   - IDT (Interrupt Descriptor Table)
   - PMM (Physical Memory Manager)
   - VMM (Virtual Memory Manager with paging)
   - Heap allocator
   - Process scheduler
   - System calls
4. Enables interrupts and starts shell

### Memory Layout

```
Address Range          | Usage
-----------------------|----------------------
0x00000000 - 0x00000FFF| Real mode IVT (unused)
0x00001000 - 0x0009FFFF| BIOS data area
0x000A0000 - 0x000BFFFF| VGA framebuffer
0x000C0000 - 0x000FFFFF| BIOS ROM
0x00100000 - 0x00FFFFFF| Kernel (loaded here)
0x01000000 - 0x07FFFFFF| Kernel heap
0x08000000 - ...       | Process memory
```

### Memory Management
- **Physical Memory Manager**: Bitmap-based allocator for 4KB blocks
- **Virtual Memory Manager**: 4-level paging (simplified to 1-level for demo)
- **Heap**: Best-fit allocator with block coalescing

### Process Scheduling
- Round-robin scheduling with 10ms time slices
- Simple process control block (PCB) structure
- Context switching saves/restores all general-purpose registers

### System Calls
| Number | Name      | Function |
|--------|-----------|----------|
| 1      | SYS_WRITE | Write to console |
| 2      | SYS_READ  | Read from keyboard |
| 20     | SYS_GETPID| Get current process ID |
| 60     | SYS_EXIT  | Terminate current process |

## Debugging

### Serial Output
All kernel output is sent to COM1 (0x3F8) in addition to VGA. Connect with QEMU:
```bash
qemu-system-x86_64 -cdrom myos.iso -serial stdio
```

### GDB Debugging
1. Start QEMU with GDB server:
   ```bash
   qemu-system-x86_64 -cdrom myos.iso -s -S
   ```
2. Connect GDB:
   ```bash
   gdb myos.bin
   (gdb) target remote localhost:1234
   (gdb) break kernel_main
   (gdb) continue
   ```

### QEMU Monitor
Press `Ctrl+Alt+2` to access QEMU monitor. Useful commands:
- `info registers` - Show CPU state
- `info mem` - Show virtual memory mapping
- `info tlb` - Show TLB contents
- `x/10i $rip` - Disassemble 10 instructions

## Common Issues and Solutions

### "grub-mkrescue: command not found"
**Solution:** Install GRUB utilities:
```bash
sudo apt-get install grub-pc-bin grub-common xorriso
```

### "undefined reference to `outb`"
**Solution:** Ensure all inline assembly functions are properly defined in `serial.h`

### Triple fault on boot
**Possible causes:**
- GDT not loaded before enabling interrupts
- IDT entry not set for exception handler
- Paging enabled without identity mapping kernel
- Stack overflow (increase stack size in boot.S)

### Page fault after enabling paging
**Solution:** Identity map the first 16MB before enabling paging:
```c
for (uintptr_t addr = 0; addr < 0x1000000; addr += 0x1000) {
    vmm_map_page((void*)addr, (void*)addr, PAGE_PRESENT | PAGE_WRITE);
}
```

### Keyboard not working
**Solution:** Ensure PIC is properly remapped and keyboard IRQ (IRQ1) is enabled:
```c
outb(0x21, inb(0x21) & 0xFD);  /* Enable IRQ1 */
```

## Extending the OS

### Adding a New System Call
1. Add syscall number in `syscall.h`
2. Implement handler in `handle_syscall()` function
3. Add wrapper function for userspace programs

### Adding a New Driver
1. Create `driver_name.c/h` in `src/kernel/`
2. Implement init function
3. Add IRQ handler if needed
4. Call init from `kernel_main()`

### Supporting ELF Executables
1. Add ELF parser in `src/kernel/elf.c`
2. Implement userspace memory mapping
3. Add privilege level switching (ring 3)

## Performance Considerations

- **Timer Resolution**: Current 10ms timer interrupt (configurable in PIC)
- **Heap Allocator**: O(n) allocation time - optimize with free lists
- **Scheduler**: Simple round-robin - add priority queues for better performance
- **VGA Output**: Direct memory access - no locking (assumes single core)

## Limitations

- No SMP support (single-core only)
- No filesystem (no disk drivers)
- No networking stack
- Limited hardware support (PS/2 keyboard only, no USB)
- No userspace protection (all code runs in ring 0)
- No dynamic module loading

## Future Improvements

- [ ] Add filesystem (FAT32 or ext2)
- [ ] Implement ATA/PATA driver for disk I/O
- [ ] Add ELF loader for userspace programs
- [ ] Implement virtual memory with full 4-level paging
- [ ] Add proper keyboard driver with scancode translation
- [ ] Implement POSIX-like syscalls
- [ ] Add simple GUI with VESA framebuffer
- [ ] Implement spinlocks for SMP support
- [ ] Add network stack (e1000 driver)

## Resources

### Books
- *Operating Systems: Design and Implementation* - Tanenbaum
- *Modern Operating Systems* - Tanenbaum
- *The Linux Kernel Architecture* - Mauerer

### Online Resources
- [OSDev Wiki](https://wiki.osdev.org/) - Bible of OS development
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot2/)
- [Intel 64 and IA-32 Architectures Manual](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)

### Reference Implementations
- [Linux Kernel](https://github.com/torvalds/linux)
- [xv6](https://github.com/mit-pdos/xv6-riscv) (educational OS)
- [ToaruOS](https://github.com/klange/toaruos)

## License

This project is released under the MIT License. See below:

```
MIT License

Copyright (c) 2025 MyOS Contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

## Contributing

While this is an educational project, contributions are welcome! Areas that need improvement:

- Bug fixes and stability improvements
- Better documentation and comments
- Additional drivers (AHCI, USB, sound)
- Performance optimizations
- Porting to other architectures (ARM, RISC-V)

## Acknowledgments

- GRUB developers for multiboot specification
- OSDev Wiki community for invaluable resources
- QEMU team for excellent emulation
- All open source OS developers who share their knowledge

## Contact

For questions or suggestions, please open an issue on the project repository.

---

**Disclaimer**: This OS is for educational purposes only. It's not intended for production use. There are known bugs and missing security features. Use at your own risk.
```

## Additional README Sections for Specific Audiences

### For Students
```markdown
## Learning Path

If you're using this code to learn OS development, here's the suggested order to study:

1. **Boot Process** (`boot.S`, `linker.ld`)
   - Learn how GRUB loads your kernel
   - Understand the switch from 32-bit to 64-bit mode

2. **Interrupts** (`idt.c`, `gdt.c`)
   - Study IDT/GDT initialization
   - Write your own interrupt handlers

3. **Memory Management** (`paging.c`, `heap.c`)
   - Implement a better physical allocator (buddy allocator)
   - Add page fault handler with demand paging

4. **Processes** (`scheduler.c`)
   - Add process priorities
   - Implement wait queues for blocking operations

5. **System Calls** (`syscall.c`)
   - Add more syscalls (open, read, write)
   - Implement copy_to_user/copy_from_user

6. **Drivers** (`serial.c`, extend for keyboard)
   - Add ATA driver for disk I/O
   - Implement a simple filesystem
```

### Performance Profiling
```markdown
## Profiling the Kernel

### Measure Context Switch Time
```c
uint64_t start = read_timestamp();
yield();
uint64_t end = read_timestamp();
printf("Switch time: %d cycles\n", end - start);
```

### Track Memory Usage
```c
printf("Free physical memory: %d KB\n", pmm_get_free_memory() / 1024);
printf("Heap allocated: %d bytes\n", heap_get_allocated());
```

### Enable QEMU Tracing
```bash
qemu-system-x86_64 -cdrom myos.iso -trace events=/tmp/trace-events
```
```

This README provides comprehensive documentation covering installation, usage, architecture, debugging, and extending the OS. It's structured to be useful for both beginners learning OS development and experienced developers who want to understand the implementation details.
#!/bin/bash
make clean
make all
make run
