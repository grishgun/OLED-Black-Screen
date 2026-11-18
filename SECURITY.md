# Security / Безопасность

**English**

OLED Black Screen is a small Win32 utility distributed as open source. It does not access the network or intentionally collect user data. It temporarily installs keyboard/mouse low-level hooks *only while the black overlay is active* to detect an action that exits it; input is not logged.

- The release EXE is **unsigned**. The source code and build workflow are provided for independent review.
- The black overlay is **not a privacy screen or lock screen**. Do not use it as an alternative to locking Windows; someone can dismiss it with a key.
- Please report security issues privately to the repository owner using GitHub's private vulnerability reporting feature, if enabled. Otherwise open an issue without posting exploit details or personal information.
- This initial preview has not been tested on a real Windows desktop; use at your own discretion.

**Русский**

OLED Black Screen — небольшая Win32-утилита с открытым кодом. Она не обращается в сеть и не собирает данные пользователя. Только на время чёрного экрана активируются низкоуровневые обработчики клавиатуры и мыши, которые реагируют на действие для выхода. Ввод не записывается.

- EXE **не подписан цифровой подписью**. Исходники и процесс сборки открыты для проверки.
- Чёрный экран **не заменяет блокировку Windows и не защищает конфиденциальность**: его можно убрать нажатием любой клавиши.
- О проблемах безопасности сообщайте владельцу репозитория через приватную отправку уязвимостей GitHub (если эта функция включена). В противном случае создайте Issue без публикации подробностей эксплуатации и персональных данных.
- Предварительная версия ещё не проверена в реальном сеансе Windows; используйте её с учётом этого ограничения.