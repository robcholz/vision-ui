import time

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# -----------------------------
# parameters
# -----------------------------
speed = 90.0
dt_ms = 16.6667


def animation_exp(pos, pos_trg, speed, delta_ms):
    if pos == pos_trg:
        return pos

    if abs(pos - pos_trg) <= 1.0:
        return pos_trg

    frame_ms = delta_ms if delta_ms > 0.0 else 16.6667
    frame_ratio = frame_ms / 16.6667
    step = (pos_trg - pos) / (100.0 - speed) * frame_ratio
    return pos + step

def animation_exp_smoother(pos, pos_trg, speed, delta_ms):
    if pos == pos_trg:
        return pos

    frame_ms = delta_ms if delta_ms > 0.0 else 16.6667
    dt = frame_ms / 1000.0

    duration = max(0.05, (1.0 - speed / 100.0) * 0.25)

    dist = abs(pos_trg - pos)
    if dist < 0.001:
        return pos_trg

    p = min(1.0, dt / duration)

    # smootherstep (quintic smoothstart/smoothstop)
    s = p*p*p * (p*(6*p - 15) + 10)

    return pos + (pos_trg - pos) * s


DAMPING_RATIO = 0.5 # ζ: overshoot 越小越平，越大越粘
BASE_NAT_FREQ = 8.0  # ω_n base，自然频率
NAT_FREQ_PER_SPEED = 0.2  # speed 对 ω_n 的影响


def animation_2nd_ode(pos, pos_trg, speed, delta_ms):
    if not hasattr(animation_exp, "vel"):
        animation_exp.vel = 0.0

    v = animation_exp.vel
    dt = delta_ms / 1000.0

    omega_n = BASE_NAT_FREQ + NAT_FREQ_PER_SPEED * speed
    zeta = DAMPING_RATIO

    a = -2.0 * zeta * omega_n * v - (omega_n ** 2) * (pos - pos_trg)

    v = v + a * dt
    x = pos + v * dt

    animation_exp.vel = v
    return x


def animation_do(pos, pos_trg, speed, delta_ms):
    return animation_2nd_ode(pos, pos_trg, speed, delta_ms)


step_input = 0.0  # step function (SPACE = 1, release = 0)
step_input_scale = 100.0
pos = 0.0
t_total = 0.0

ts = []
ys = []
us = []  # store step input history

last_time = time.time()

plt.ion()
fig, ax = plt.subplots()

line_resp, = ax.plot([], [], lw=2, label="response (pos)")
line_step, = ax.plot([], [], "--", lw=1.5, label="step input")

ax.set_ylim(-50, 150)
ax.set_xlim(0, 2000)
ax.set_xlabel("time (ms)")
ax.set_ylabel("value")
ax.set_title("Step Input vs Response")
ax.grid(True)
ax.legend()


def on_key_press(event):
    global step_input
    if event.key == " ":
        step_input = 1.0 * step_input_scale
        print("STEP = 1 (SPACE DOWN)")


def on_key_release(event):
    global step_input
    if event.key == " ":
        step_input = 0.0
        print("STEP = 0 (SPACE UP)")


fig.canvas.mpl_connect("key_press_event", on_key_press)
fig.canvas.mpl_connect("key_release_event", on_key_release)


def update(frame):
    global pos, t_total, last_time, ts, ys, us, step_input

    now = time.time()
    delta_ms = (now - last_time) * 1000.0
    if delta_ms <= 0:
        delta_ms = dt_ms
    last_time = now

    t_total += delta_ms

    pos = animation_do(pos, step_input, speed, delta_ms)

    ts.append(t_total)
    ys.append(pos)
    us.append(step_input)

    # update lines
    line_resp.set_data(ts, ys)
    line_step.set_data(ts, us)

    # auto scroll x-axis
    if t_total > 2000:
        ax.set_xlim(t_total - 2000, t_total + 10)

    return line_resp, line_step


ani = FuncAnimation(fig, update, interval=16, blit=False)
plt.show(block=True)
