import numpy as np
import matplotlib.pyplot as plt
from scipy.special import fresnel
import os

filename = "fresnel_profile_cpp.txt"  # Файл, сохранённый C++ программой

if not os.path.exists(filename):
    print(f"Файл {filename} не найден. Запустите C++ программу и нажмите 'S' для сохранения профиля.")
    exit(1)

# Чтение параметров из заголовка файла
params = {}
with open(filename, 'r') as f:
    for line in f:
        if line.startswith('# lambda='):
            params['lambda'] = float(line.split('=')[1])
        elif line.startswith('# b='):
            params['b'] = float(line.split('=')[1])
        elif line.startswith('# z='):
            params['z'] = float(line.split('=')[1])
        elif line.startswith('# xRange='):
            params['xRange'] = float(line.split('=')[1])
        elif not line.startswith('#'):
            break  # дальше идут данные

# Загрузка данных (пропуская комментарии)
data = np.loadtxt(filename, comments='#')
x_cpp = data[:, 0]
I_cpp = data[:, 1]

# Если параметры не прочитались (старый формат файла), используем значения по умолчанию
if 'lambda' not in params:
    print("Параметры не найдены в файле. Используются значения по умолчанию: lambda=515e-9, b=0.5e-3, z=0.15")
    params['lambda'] = 515e-9
    params['b'] = 0.5e-3
    params['z'] = 0.15
    params['xRange'] = x_cpp[-1] - x_cpp[0]  # примерный диапазон

lambda_ = params['lambda']
b = params['b']
z = params['z']
xRange = params['xRange']

print(f"Параметры из C++: lambda = {lambda_*1e9:.1f} нм, b = {b*1000:.2f} мм, z = {z*100:.1f} см, xRange = {xRange*1000:.2f} мм")

# Расчёт профиля в Python с теми же параметрами
num_points_cpp = len(x_cpp)
x_py = np.linspace(-xRange/2, xRange/2, num_points_cpp)
half_width = b / 2.0
factor = np.sqrt(2.0 / (lambda_ * z))

u1 = factor * (x_py - half_width)
u2 = factor * (x_py + half_width)

C1, S1 = fresnel(u1)
C2, S2 = fresnel(u2)

I_py = (C2 - C1)**2 + (S2 - S1)**2
I_py /= np.max(I_py)  # нормировка, как в C++

# Интерполяция на точки C++ (на случай разной сетки)
I_py_at_cpp = np.interp(x_cpp, x_py, I_py)
diff = I_py_at_cpp - I_cpp

print(f"Максимальная абсолютная разница: {np.max(np.abs(diff)):.2e}")

# Построение графиков
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(12, 5))
ax1.plot(x_cpp * 1000, I_cpp, label='C++ (наша реализация)')
ax1.plot(x_cpp * 1000, I_py_at_cpp, '--', label='Python (scipy)')
ax1.set_xlabel('x, мм')
ax1.set_ylabel('Нормированная интенсивность')
ax1.set_title('Сравнение профилей')
ax1.legend()
ax1.grid(True)

ax2.plot(x_cpp * 1000, diff)
ax2.set_xlabel('x, мм')
ax2.set_ylabel('Разница (Python - C++)')
ax2.set_title('Абсолютная разница')
ax2.grid(True)

plt.tight_layout()
plt.show()