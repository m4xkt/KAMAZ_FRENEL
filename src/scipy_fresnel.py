import numpy as np
import matplotlib.pyplot as plt
from scipy.special import fresnel
import os

# Параметры (должны совпадать с C++)
lambda_ = 515e-9
b = 0.5e-3
z = 0.15

# Расчёт Python
x_py = np.linspace(-1e-3, 1e-3, 8000)
half_width = b/2
factor = np.sqrt(2/(lambda_*z))
u1 = factor*(x_py - half_width)
u2 = factor*(x_py + half_width)
C1, S1 = fresnel(u1)
C2, S2 = fresnel(u2)
I_py = (C2-C1)**2 + (S2-S1)**2
I_py /= np.max(I_py)

# Загрузка данных C++
filename = 'fresnel_profile_cpp.txt'
if not os.path.exists(filename):
    print(f"Файл {filename} не найден!")
    exit(1)

data = np.loadtxt(filename, comments='#')
if data.ndim == 1:
    data = data.reshape(1, -1)
x_cpp = data[:,0]
I_cpp = data[:,1]

# Удаляем NaN/Inf
valid = np.isfinite(I_cpp)
x_cpp = x_cpp[valid]
I_cpp = I_cpp[valid]

if len(x_cpp) == 0:
    print("Нет корректных данных в C++ файле")
    exit(1)

# Интерполяция Python на точки C++
I_py_at_cpp = np.interp(x_cpp, x_py, I_py)
diff = I_py_at_cpp - I_cpp

print(f"Максимальная абсолютная разница: {np.max(np.abs(diff)):.2e}")

# Построение графиков
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12,5))
ax1.plot(x_cpp*1000, I_cpp, label='C++')
ax1.plot(x_cpp*1000, I_py_at_cpp, '--', label='Python scipy')
ax1.set_xlabel('x (мм)')
ax1.set_ylabel('Интенсивность')
ax1.legend()
ax1.grid(True)

ax2.plot(x_cpp*1000, diff)
ax2.set_xlabel('x (мм)')
ax2.set_ylabel('Разница (Python - C++)')
ax2.grid(True)

plt.tight_layout()
plt.show()