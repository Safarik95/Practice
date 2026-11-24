# Музыкальная программа

Учебная практика, вариант 2 «Музыкальная программа». Оконное приложение на C++20 и Qt 6 Widgets.

## Окружение

- Windows 10
- Qt 6.12.1, комплект MinGW 15.1.0 64-bit
- CMake 3.30.5, Ninja 1.12.1
- Qt Creator 21.0.0
- Стандарт языка: C++20

## Сборка и запуск

1. Открыть `CMakeLists.txt` в Qt Creator.
2. Выбрать комплект Desktop Qt 6.12.1 MinGW 64-bit.
3. Собрать проект и запустить цель `MusicProgram`.

## Тесты

Тесты написаны на Qt Test и зарегистрированы в CTest. Запуск в Qt Creator: Tools → Tests → Run All Tests. Запуск из консоли в каталоге сборки:

```
ctest --output-on-failure
```
