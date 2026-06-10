import serial
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import collections
import numpy as np

# --- CONFIGURATION ---
SERIAL_PORT = 'rfc2217://localhost:4000' 
BAUD_RATE = 115200
WINDOW_SIZE = 100  # Smaller window = faster, more responsive scaling

# Global containers
queues = []
lines = []
axes = []
initialized = False

try:
    ser = serial.serial_for_url(SERIAL_PORT, baudrate=BAUD_RATE, timeout=0.1)
    print(f"Connected to {SERIAL_PORT}")
except Exception as e:
    print(f"Error: {e}")
    exit()

fig = plt.figure(figsize=(10, 8))

def setup_plots(num_signals):
    global initialized, queues, lines, axes
    print(f"Detecting {num_signals} signals. Initializing plots...")
    
    # Clear existing figure
    fig.clf()
    queues = [collections.deque([0.0] * WINDOW_SIZE, maxlen=WINDOW_SIZE) for _ in range(num_signals)]
    axes = [fig.add_subplot(num_signals, 1, i + 1) for i in range(num_signals)]
    
    colors = ['forestgreen', 'royalblue', 'orange', 'red', 'purple', 'brown']
    lines = []
    
    for i, ax in enumerate(axes):
        line, = ax.plot([0]*WINDOW_SIZE, color=colors[i % len(colors)], label=f"Signal {i+1}")
        lines.append(line)
        ax.legend(loc="upper right")
        ax.grid(True, alpha=0.3)
    
    fig.tight_layout()
    initialized = True

def update(frame):
    global initialized
    if ser.in_waiting > 0:
        line_str = ser.readline().decode('utf-8', errors='ignore').strip()
        try:
            values = [float(v) for v in line_str.split(',') if v.strip()]
            num_signals = len(values)

            # Auto-initialize on first valid data packet
            if not initialized and num_signals > 0:
                setup_plots(num_signals)
                return lines

            if initialized and num_signals == len(queues):
                for i in range(num_signals):
                    queues[i].append(values[i])
                    lines[i].set_ydata(queues[i])
                    
                    # Dynamic Scaling per Plot
                    d_min, d_max = min(queues[i]), max(queues[i])
                    margin = (d_max - d_min) * 0.15 if d_max != d_min else 1.0
                    axes[i].set_ylim(d_min - margin, d_max + margin)
            
        except (ValueError, IndexError):
            pass 
            
    return lines

ani = FuncAnimation(fig, update, interval=20, blit=False, cache_frame_data=False)
plt.show()
ser.close