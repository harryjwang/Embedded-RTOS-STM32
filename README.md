# Custom Embedded STM32 RTOS

This project implements a lightweight real-time operating system (RTOS) for the STM32F401RE microcontroller, designed to demonstrate the fundamentals of task management, scheduling, and low-level hardware control in embedded systems.

The RTOS is built around a cooperative task scheduler, with context switching implemented using the ARM Cortex-M PendSV interrupt. Each task is managed through a Task Control Block (TCB), and task state is preserved using a manually constructed exception frame. The system also includes basic dynamic memory management to allocate and free resources at runtime.

Key features include:

- Task Scheduling: Cooperative, round-robin style task switching.

- Context Switching: Implemented in ARM Thumb assembly using the PendSV handler.

- Task Management APIs: Support for task creation, yielding, exiting, and querying task information.

- Memory Management: Dynamic memory allocation for task stacks and kernel objects.

- STM32 Integration: Built using STM32CubeIDE with STM32 HAL drivers.

- This project provides a foundation for experimenting with multitasking in embedded systems and serves as a stepping stone for more advanced RTOS features such as preemptive scheduling, IPC mechanisms, and device drivers.
