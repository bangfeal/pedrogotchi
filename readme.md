Pedrogotchi!!! es una mascota virtual work in progress para la ESP32S3.
Todo el código de la branch actual es meramente proof of concept y no
debería ser tomado en serio, por nadie. Cualquier semejanza con personas
reales es una coincidencia.

Tu mascota, Pedro, un oficinista de Madrid, tendrá una vida tan buena como
tú le puedas dar. Puedes ver sus niveles de felicidad, hambre, y frío usando
la opción de la báscula en el menú, la cuál se puede acceder pulsando el botón
rojo conectado a GPIO40 para cambiar de opción, y el botón azul conectado a
GPIO41 para finalmente seleccionar una. Usa el botón blanco, conectado a GPIO42
para volver al menú inicial.

Pedrogotchi!!! es muy sensible a la luz. Cuando se apagan las luces, Pedro duerme.
Su barra de frío también es muy importante. Usando datos recopilados de
OpenWeatherMap a través del WiFi de la ESP32S3, Pedro puede sentir el frío tanto
como tú.

Conecta un buzzer a GPIO35 para disfrutar esos clásicos beeps por click que tanto
te recuerdan a tu infancia. En esta última versión del software de Pedrogotchi!!!
aún faltan muchas funciones, pero quién sabe, quizás algún día lleguen.

Pedrogotchi!!! está diseñado para servir con un fotoresistor específico, y con una
pantalla LED ST7789 de 240x240 píxeles de resolución.
