import tkinter as tk
from tkinter import messagebox, filedialog, ttk

class RTOSConfigGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("RTOS Configuration Generator")

        # Hardcoded Kernel Version
        self.kernel_version = "0.1.0"

        # Variables for thread configuration
        self.nthreads = tk.IntVar(value=3)
        self.stack_sizes = []
        self.thread_names = []

        # Task-specific PIDs and stack sizes
        self.idle_task_pid = tk.IntVar(value=0)
        self.idle_task_stack_size = tk.IntVar(value=128)
        self.tim_handler_pid = tk.IntVar(value=1)
        self.tim_handler_stack_size = tk.IntVar(value=128)

        # Scheduler Configuration
        self.num_priorities = tk.IntVar(value=5)
        self.time_slice = tk.IntVar(value=10)
        self.scheduler_mode = tk.StringVar(value="Preemptive")

        # Communication Configuration
        self.message_queue_enabled = tk.BooleanVar(value=False)
        self.mailbox_enabled = tk.BooleanVar(value=False)
        self.mailbox_ack_enabled = tk.BooleanVar(value=False)
        self.pipe_type = tk.StringVar(value="Simple")
        self.num_message_buffers = tk.IntVar(value=5)

        # Synchronization Configuration
        self.condition_variables_enabled = tk.BooleanVar(value=True)

        # Trace Configuration
        self.trace_enabled = tk.BooleanVar(value=True)
        self.trace_buffer_size = tk.IntVar(value=512)

        self.build_gui()

    def build_gui(self):
        # General Section
        general_frame = ttk.LabelFrame(self.root, text="General", padding=10)
        general_frame.grid(row=0, column=0, columnspan=2, padx=10, pady=5, sticky="ew")
        tk.Label(general_frame, text=f"Kernel Version: {self.kernel_version}", font=("Arial", 10, "bold")).grid(row=0, column=0, sticky="w")

        # Threads Section
        threads_frame = ttk.LabelFrame(self.root, text="Threads", padding=10)
        threads_frame.grid(row=1, column=0, columnspan=2, padx=10, pady=5, sticky="ew")
        tk.Label(threads_frame, text="Number of Threads:").grid(row=0, column=0, sticky="w")
        tk.Spinbox(threads_frame, from_=1, to=32, textvariable=self.nthreads, command=self.update_thread_entries).grid(row=0, column=1)

        self.thread_frame = tk.Frame(threads_frame)
        self.thread_frame.grid(row=1, columnspan=2, pady=10)
        self.update_thread_entries()

        # Scheduler Section
        scheduler_frame = ttk.LabelFrame(self.root, text="Scheduler", padding=10)
        scheduler_frame.grid(row=2, column=0, columnspan=2, padx=10, pady=5, sticky="ew")
        self.build_scheduler_section(scheduler_frame)

        # Communication Section
        communication_frame = ttk.LabelFrame(self.root, text="Communication", padding=10)
        communication_frame.grid(row=3, column=0, columnspan=2, padx=10, pady=5, sticky="ew")
        self.build_communication_section(communication_frame)

        # Synchronization Section
        sync_frame = ttk.LabelFrame(self.root, text="Synchronization", padding=10)
        sync_frame.grid(row=4, column=0, columnspan=2, padx=10, pady=5, sticky="ew")
        self.build_sync_section(sync_frame)

        # Trace Section
        trace_frame = ttk.LabelFrame(self.root, text="Trace", padding=10)
        trace_frame.grid(row=5, column=0, columnspan=2, padx=10, pady=5, sticky="ew")
        self.build_trace_section(trace_frame)

        # Generate Button
        tk.Button(self.root, text="Generate Config", command=self.generate_config).grid(row=6, columnspan=2, pady=10)

    def build_scheduler_section(self, frame):
        tk.Label(frame, text="Idle Task PID:").grid(row=0, column=0, sticky="w")
        tk.Spinbox(frame, from_=0, to=255, textvariable=self.idle_task_pid).grid(row=0, column=1)

        tk.Label(frame, text="Idle Task Stack Size:").grid(row=1, column=0, sticky="w")
        tk.Spinbox(frame, from_=64, to=1024, textvariable=self.idle_task_stack_size).grid(row=1, column=1)

        tk.Label(frame, text="Timer Handler PID:").grid(row=2, column=0, sticky="w")
        tk.Spinbox(frame, from_=1, to=255, textvariable=self.tim_handler_pid).grid(row=2, column=1)

        tk.Label(frame, text="Timer Handler Stack Size:").grid(row=3, column=0, sticky="w")
        tk.Spinbox(frame, from_=64, to=1024, textvariable=self.tim_handler_stack_size).grid(row=3, column=1)

        tk.Label(frame, text="Scheduler Mode:").grid(row=4, column=0, sticky="w")
        tk.OptionMenu(frame, self.scheduler_mode, "Preemptive", "Cooperative").grid(row=4, column=1)

        tk.Label(frame, text="Time Slice (ms):").grid(row=5, column=0, sticky="w")
        tk.Spinbox(frame, from_=1, to=100, textvariable=self.time_slice).grid(row=5, column=1)

        tk.Label(frame, text="Number of Priorities:").grid(row=6, column=0, sticky="w")
        tk.Spinbox(frame, from_=1, to=32, textvariable=self.num_priorities).grid(row=6, column=1)

    def build_communication_section(self, frame):
        tk.Checkbutton(frame, text="Enable Message Queue", variable=self.message_queue_enabled).grid(row=0, column=0, sticky="w")
        tk.Checkbutton(frame, text="Enable Mailbox", variable=self.mailbox_enabled).grid(row=0, column=1, sticky="w")
        tk.Checkbutton(frame, text="Enable Mailbox ACK (Extended Rendez-vous)", variable=self.mailbox_ack_enabled).grid(row=1, columnspan=2, sticky="w")

        tk.Label(frame, text="Number of Message Buffers:").grid(row=2, column=0, sticky="w")
        tk.Spinbox(frame, from_=1, to=32, textvariable=self.num_message_buffers).grid(row=2, column=1)

        tk.Radiobutton(frame, text="Simple Pipe", variable=self.pipe_type, value="Simple").grid(row=3, column=0, sticky="w")
        tk.Radiobutton(frame, text="Extended Pipe", variable=self.pipe_type, value="Extended", state="normal" if self.condition_variables_enabled.get() else "disabled").grid(row=3, column=1, sticky="w")

    def build_sync_section(self, frame):
        tk.Checkbutton(frame, text="Enable Condition Variables", variable=self.condition_variables_enabled, command=self.update_pipe_options).grid(row=0, column=0, sticky="w")

    def build_trace_section(self, frame):
        tk.Checkbutton(frame, text="Enable Trace", variable=self.trace_enabled).grid(row=0, column=0, sticky="w")
        tk.Label(frame, text="Trace Buffer Size:").grid(row=1, column=0, sticky="w")
        tk.Spinbox(frame, from_=128, to=1024, textvariable=self.trace_buffer_size).grid(row=1, column=1)

    def update_thread_entries(self):
        for widget in self.thread_frame.winfo_children():
            widget.destroy()

        self.stack_sizes = [tk.IntVar(value=128) for _ in range(self.nthreads.get())]
        self.thread_names = [tk.StringVar(value=f"Thread_{i}") for i in range(self.nthreads.get())]

        for i in range(self.nthreads.get()):
            tk.Label(self.thread_frame, text=f"Thread {i} Name:").grid(row=i, column=0, sticky="w")
            tk.Entry(self.thread_frame, textvariable=self.thread_names[i]).grid(row=i, column=1)

    def update_pipe_options(self):
        state = "normal" if self.condition_variables_enabled.get() else "disabled"
        if state == "disabled":
            self.pipe_type.set("Simple")

    def generate_config(self):
        config_content = self.build_config_content()
        file_path = filedialog.asksaveasfilename(defaultextension=".h", filetypes=[("Header Files", "*.h")])
        if file_path:
            with open(file_path, "w") as f:
                f.write(config_content)
            messagebox.showinfo("Success", "Configuration file generated successfully!")

    def build_config_content(self):
        externs = "\n".join([f"extern STACK {name.get()}[{size.get()}];" for name, size in zip(self.thread_names, self.stack_sizes)])
        return f"""
#ifndef INC_K_CONFIG_H_
#define INC_K_CONFIG_H_

#define KVERSION "{self.kernel_version}"

{externs}

#endif /* INC_K_CONFIG_H_ */
"""

if __name__ == "__main__":
    root = tk.Tk()
    app = RTOSConfigGUI(root)
    root.mainloop()
