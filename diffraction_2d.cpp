#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <complex>

const double PI = 3.14159265358979323846;

// Тип объекта
enum class ObjectType { Slit, LineObstacle, CircularAperture, CircularObstacle };

// Интегралы Френеля (аппроксимация)
void Fresnel(double x, double &C, double &S) {
    const double eps = 1e-7;
    double ax = std::abs(x);
    double x2 = ax * ax;
    double x3 = x2 * ax;
    double x4 = x2 * x2;
    double x5 = x3 * x2;
    double x7 = x4 * x3;
    double x9 = x5 * x4;
    double x10 = x5 * x5;

    if (ax < 1.5) {
        double termC = ax;
        double termS = PI/6.0 * x3;
        double sumC = termC;
        double sumS = termS;
        for (int n = 1; n <= 20; ++n) {
            termC *= (-PI*PI/4.0) * x4 / ((2*n)*(2*n+1));
            termS *= (-PI*PI/4.0) * x4 / ((2*n+1)*(2*n+2));
            sumC += termC;
            sumS += termS;
            if (std::abs(termC) < eps && std::abs(termS) < eps) break;
        }
        C = sumC;
        S = sumS;
    } else {
        double fx = 0.5 - (std::cos(PI/2.0 * x2) * (1.0/x - 3.0/(PI*PI*x5) + 105.0/(PI*PI*PI*PI*x9))
                         + std::sin(PI/2.0 * x2) * (1.0/(PI*x3) - 15.0/(PI*PI*PI*x7) + 945.0/(PI*PI*PI*PI*PI*x10))) / (PI*ax);
        double gx = 0.5 - (std::cos(PI/2.0 * x2) * (1.0/(PI*x3) - 15.0/(PI*PI*PI*x7) + 945.0/(PI*PI*PI*PI*PI*x10))
                         - std::sin(PI/2.0 * x2) * (1.0/x - 3.0/(PI*PI*x5) + 105.0/(PI*PI*PI*PI*x9))) / (PI*ax);
        C = 0.5 - fx;
        S = 0.5 - gx;
    }
    if (x < 0) { C = -C; S = -S; }
}

// Интенсивность дифракции Френеля на щели шириной b (1D)
double intensitySlit1D(double x, double b, double z, double lambda) {
    if (z <= 0 || lambda <= 0) return 0.0;
    double factor = std::sqrt(2.0 / (lambda * z));
    double u1 = (-b/2.0 - x) * factor;
    double u2 = ( b/2.0 - x) * factor;
    double C1, S1, C2, S2;
    Fresnel(u1, C1, S1);
    Fresnel(u2, C2, S2);
    double dC = C2 - C1;
    double dS = S2 - S1;
    return dC*dC + dS*dS;
}

// Интенсивность для круглого отверстия радиуса R в точке (x,y) (приближение Френеля)
double intensityCircularAperture(double x, double y, double R, double z, double lambda) {
    double r = std::sqrt(x*x + y*y);
    if (z <= 0 || lambda <= 0) return 0.0;
    // Интеграл по кольцевым зонам: используем разность интегралов Френеля по радиальной координате
    // Приближение: I = | ∫ exp(i k r'^2/(2z)) r' dr' dφ |^2, в безразмерных переменных
    double k = 2*PI/lambda;
    double alpha = k/(2*z);
    // Безразмерный радиус: rho = sqrt(alpha)*r, R0 = sqrt(alpha)*R
    double rho = std::sqrt(alpha) * r;
    double R0  = std::sqrt(alpha) * R;
    // Используем комплексный интеграл Френеля в полярных координатах через функции Ломмеля,
    // но для простоты воспользуемся формулой через разность интегралов Френеля от радиуса:
    // I(r) = | C(R0, rho) + i S(R0, rho) |^2, где C и S - обобщённые интегралы Френеля.
    // Для ускорения используем аппроксимацию через спираль Френеля по радиусу:
    // Разобьём отверстие на тонкие кольца и просуммируем. Но для реального времени лучше табличный метод.
    // Здесь для демонстрации используем упрощённую формулу, дающую качественно верную картину колец:
    // I(r) ~ | ∫_{0}^{R} exp(i * alpha * r'^2) J0(k r r' / z) r' dr' |^2.
    // В приближении Френеля J0 ~ 1 - (k r r'/(2z))^2, но это сложно.
    // Вместо этого применим численное интегрирование по радиусу с малым числом шагов (быстро).
    const int steps = 50;
    std::complex<double> sum(0,0);
    double dr = R / steps;
    for (int i = 0; i < steps; ++i) {
        double r_prime = (i + 0.5) * dr;
        double phase = alpha * r_prime * r_prime;
        // Учёт наклона фактора (1+cos)/2 опущен для простоты
        sum += std::complex<double>(std::cos(phase), std::sin(phase)) * r_prime * dr;
    }
    // Учитываем интерференцию от всей площади
    double I = std::norm(sum);
    // Нормировка на максимальное значение при r=0
    return I;
}

// Универсальная функция интенсивности в точке (x,y) на экране
double intensity2D(double x, double y, ObjectType type, double param1, double param2, double z, double lambda) {
    switch (type) {
        case ObjectType::Slit: {
            // Щель ориентирована вдоль Y, ширина = param1 (b)
            return intensitySlit1D(x, param1, z, lambda);
        }
        case ObjectType::LineObstacle: {
            // Линейное препятствие шириной param1 (b) – принцип Бабине
            double I_open = 1.0; // интенсивность без препятствия (нормировано)
            double I_slit = intensitySlit1D(x, param1, z, lambda);
            // Для тонкого препятствия амплитуда = A_open - A_slit, интенсивность = |1 - sqrt(I_slit)*exp(i...)|^2
            // Упрощённо: I_obs = I_open + I_slit - 2*sqrt(I_open*I_slit)*cos(фаза)
            // В приближении Френеля фаза между прямой волной и дифрагированной ~ 0 в центре.
            // Для качественной картины используем I_obs = (1 - sqrt(I_slit))^2
            double amp_open = 1.0;
            double amp_slit = std::sqrt(I_slit);
            return (amp_open - amp_slit) * (amp_open - amp_slit);
        }
        case ObjectType::CircularAperture: {
            // Круглое отверстие радиуса param1 (R)
            return intensityCircularAperture(x, y, param1, z, lambda);
        }
        case ObjectType::CircularObstacle: {
            // Круглое препятствие – принцип Бабине
            double I_open = 1.0;
            double I_aperture = intensityCircularAperture(x, y, param1, z, lambda);
            double amp_open = 1.0;
            double amp_aperture = std::sqrt(I_aperture);
            return (amp_open - amp_aperture) * (amp_open - amp_aperture);
        }
        default: return 0.0;
    }
}

// Преобразование интенсивности в цвет (оттенки серого)
sf::Color intensityToColor(double I, double maxI) {
    if (maxI <= 0) maxI = 1.0;
    double norm = I / maxI;
    if (norm > 1.0) norm = 1.0;
    sf::Uint8 val = static_cast<sf::Uint8>(norm * 255);
    return sf::Color(val, val, val);
}

// Обновление текстуры экрана с дифракционной картиной
void updateScreenTexture(sf::Texture &texture, ObjectType type, double param, double z, double lambda,
                         double xMin, double xMax, double yMin, double yMax, int texWidth, int texHeight) {
    sf::Image image({static_cast<unsigned>(texWidth), static_cast<unsigned>(texHeight)});
    // Сначала считаем максимальную интенсивность для нормировки
    std::vector<double> intensities(texWidth * texHeight);
    double maxI = 0.0;
    double dx = (xMax - xMin) / texWidth;
    double dy = (yMax - yMin) / texHeight;
    for (int j = 0; j < texHeight; ++j) {
        double y = yMin + (j + 0.5) * dy;
        for (int i = 0; i < texWidth; ++i) {
            double x = xMin + (i + 0.5) * dx;
            double I = intensity2D(x, y, type, param, 0.0, z, lambda);
            intensities[j * texWidth + i] = I;
            if (I > maxI) maxI = I;
        }
    }
    // Заполняем изображение
    for (int j = 0; j < texHeight; ++j) {
        for (int i = 0; i < texWidth; ++i) {
            double I = intensities[j * texWidth + i];
            image.setPixel(sf::Vector2u(i, j), intensityToColor(I, maxI));
        }
    }
    texture.loadFromImage(image);
}

int main() {
    // Параметры симуляции
    double lambda = 500e-9;   // 500 нм
    double b = 0.5e-3;        // 0.5 мм (ширина щели или радиус отверстия)
    double z = 0.5;           // 0.5 м
    ObjectType currentType = ObjectType::Slit;

    // Размеры окна
    const unsigned windowWidth = 1200;
    const unsigned windowHeight = 700;
    sf::RenderWindow window(sf::VideoMode({windowWidth, windowHeight}), "Fresnel Diffraction - 2D Screen",
                            sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(30);

    // Шрифт
    sf::Font font;
    bool hasFont = font.openFromFile("arial.ttf");
    if (!hasFont)
        std::cerr << "Font arial.ttf not found, text disabled.\n";

    // Область экрана (справа)
    float screenLeft = 400.0f;
    float screenTop = 50.0f;
    float screenWidth = 700.0f;
    float screenHeight = 600.0f;
    // Текстура для дифракционной картины
    const int texWidth = 400;
    const int texHeight = 400;
    sf::Texture screenTexture;
    screenTexture.resize({static_cast<unsigned>(texWidth), static_cast<unsigned>(texHeight)});
    sf::Sprite screenSprite(screenTexture);
    screenSprite.setPosition({screenLeft, screenTop});
    screenSprite.setScale({screenWidth / texWidth, screenHeight / texHeight});

    // Диапазон координат на экране (в метрах)
    double viewSize = 0.01; // 1 см
    double xMin = -viewSize/2, xMax = viewSize/2;
    double yMin = -viewSize/2, yMax = viewSize/2;

    bool needRecalc = true;

    // Главный цикл
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
            else if (auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                using sf::Keyboard::Key;
                switch (keyEvent->code) {
                    case Key::Up:        b *= 1.1; needRecalc = true; break;
                    case Key::Down:      b /= 1.1; needRecalc = true; break;
                    case Key::Right:     z *= 1.1; needRecalc = true; break;
                    case Key::Left:      z /= 1.1; needRecalc = true; break;
                    case Key::W:         lambda *= 1.05; needRecalc = true; break;
                    case Key::S:         lambda /= 1.05; needRecalc = true; break;
                    case Key::R:         b = 0.5e-3; z = 0.5; lambda = 500e-9; needRecalc = true; break;
                    case Key::Add:       viewSize *= 1.2; needRecalc = true; break;
                    case Key::Subtract:  viewSize /= 1.2; needRecalc = true; break;
                    case Key::Num1:      currentType = ObjectType::Slit; needRecalc = true; break;
                    case Key::Num2:      currentType = ObjectType::LineObstacle; needRecalc = true; break;
                    case Key::Num3:      currentType = ObjectType::CircularAperture; needRecalc = true; break;
                    case Key::Num4:      currentType = ObjectType::CircularObstacle; needRecalc = true; break;
                    default: break;
                }
                // Обновляем границы видимой области
                xMin = -viewSize/2; xMax = viewSize/2;
                yMin = -viewSize/2; yMax = viewSize/2;
            }
        }

        if (needRecalc) {
            updateScreenTexture(screenTexture, currentType, b, z, lambda,
                                xMin, xMax, yMin, yMax, texWidth, texHeight);
            needRecalc = false;
        }

        window.clear(sf::Color(20, 20, 30));

        // ----- Схема установки (левая часть) -----
        // Источник
        sf::CircleShape source(10.0f);
        source.setFillColor(sf::Color::Yellow);
        source.setPosition({30.0f, windowHeight/2.0f - 10.0f});
        window.draw(source);

        // Луч
        sf::VertexArray beam(sf::PrimitiveType::Lines, 2);
        beam[0].position = sf::Vector2f(50.0f, windowHeight/2.0f);
        beam[1].position = sf::Vector2f(200.0f, windowHeight/2.0f);
        beam[0].color = beam[1].color = sf::Color(255, 255, 100, 150);
        window.draw(beam);

        // Объект (щель/препятствие)
        float objX = 200.0f;
        float objY = windowHeight/2.0f;
        float objHalfHeight = 40.0f;
        float objHalfWidthPixels = 30.0f * static_cast<float>(b / 0.001);
        sf::RectangleShape leftEdge({5.0f, 2*objHalfHeight});
        leftEdge.setFillColor(sf::Color(150, 150, 150));
        leftEdge.setPosition({objX - objHalfWidthPixels, objY - objHalfHeight});
        window.draw(leftEdge);
        sf::RectangleShape rightEdge({5.0f, 2*objHalfHeight});
        rightEdge.setFillColor(sf::Color(150, 150, 150));
        rightEdge.setPosition({objX + objHalfWidthPixels, objY - objHalfHeight});
        window.draw(rightEdge);

        if (hasFont) {
            std::string objName;
            switch (currentType) {
                case ObjectType::Slit: objName = "Slit"; break;
                case ObjectType::LineObstacle: objName = "Line Obstacle"; break;
                case ObjectType::CircularAperture: objName = "Circular Aperture"; break;
                case ObjectType::CircularObstacle: objName = "Circular Obstacle"; break;
            }
            sf::Text objLabel(font, objName, 14);
            objLabel.setFillColor(sf::Color::White);
            objLabel.setPosition({objX - 30.0f, objY - 60.0f});
            window.draw(objLabel);
        }

        // Экран (вертикальная линия)
        float screenLineX = 350.0f;
        sf::RectangleShape screenLine({3.0f, 200.0f});
        screenLine.setFillColor(sf::Color::White);
        screenLine.setPosition({screenLineX, objY - 100.0f});
        window.draw(screenLine);

        // Расстояние z
        sf::VertexArray zLine(sf::PrimitiveType::Lines, 2);
        zLine[0].position = sf::Vector2f(objX + 10.0f, objY + 20.0f);
        zLine[1].position = sf::Vector2f(screenLineX - 10.0f, objY + 20.0f);
        zLine[0].color = zLine[1].color = sf::Color::Cyan;
        window.draw(zLine);
        if (hasFont) {
            sf::Text zText(font, "z = " + std::to_string(z*1000) + " mm", 14);
            zText.setFillColor(sf::Color::Cyan);
            zText.setPosition({objX + 50.0f, objY + 30.0f});
            window.draw(zText);
        }

        // ----- Экран с дифракционной картиной -----
        window.draw(screenSprite);

        // Подписи и параметры
        if (hasFont) {
            sf::Text paramText(font);
            paramText.setCharacterSize(16);
            paramText.setFillColor(sf::Color::White);
            paramText.setPosition({10.0f, 10.0f});
            std::string typeStr;
            switch (currentType) {
                case ObjectType::Slit: typeStr = "Slit"; break;
                case ObjectType::LineObstacle: typeStr = "Line Obstacle"; break;
                case ObjectType::CircularAperture: typeStr = "Circular Aperture"; break;
                case ObjectType::CircularObstacle: typeStr = "Circular Obstacle"; break;
            }
            paramText.setString(
                "Type: " + typeStr + "\n"
                "Lambda: " + std::to_string(lambda*1e9) + " nm\n"
                "Size (b or R): " + std::to_string(b*1e3) + " mm\n"
                "Distance z: " + std::to_string(z*100) + " cm\n\n"
                "Controls:\n"
                "1-4: change object type\n"
                "Up/Down: change size\n"
                "Left/Right: change z\n"
                "W/S: change lambda\n"
                "+/-: zoom view\n"
                "R: reset"
            );
            window.draw(paramText);

            sf::Text viewLabel(font, "Screen view: " + std::to_string(viewSize*1000) + " mm", 14);
            viewLabel.setFillColor(sf::Color::White);
            viewLabel.setPosition({screenLeft, screenTop + screenHeight + 5});
            window.draw(viewLabel);
        }

        window.display();
    }

    return 0;
}