# M10. Actividad Colaborativa 

## (CVE-2022-21894): Secure Boot Security Feature Bypass Vulnerability

![Rootkit-UEFI-Malware-BlackLotus](https://github.com/jmnava/mrev/assets/6135174/85a88ba4-af8a-444a-99f8-2ef4bf98f670)

En esta investigación vamos a analizar más en profundidad la explotación de la vulnerabilidad CVE-2022-21894 de la que hace uso el bootkit BlackLotus. Lo primero de todo es presentar en qué consiste el concepto de Secure Boot. Tras esto, veremos en qué consiste el bootkit BlackLotus y, por último, comentaremos más en profundidad cómo funciona la explotación de este CVE.

### 1- Secure Boot ¿Qué es?

Secure Boot es una característica de seguridad disponible en la mayoría de los equipos modernos que ayuda a asegurar que solo se ejecute software confiable durante el proceso de arranque del sistema operativo. El objetivo principal de Secure Boot es prevenir que software malicioso se cargue y se ejecute cuando el sistema está arrancando. Esto se hace verificando que todos los componentes del software estén firmados con una clave de confianza.

**Elementos:**

- **Claves de seguridad**: Secure Boot utiliza un conjunto de claves criptográficas para validar el software que se va a ejecutar. Estas claves están almacenadas en el firmware del sistema.
- **Firma digital**: Cada pieza de software que se carga durante el arranque está firmada digitalmente. El firmware UEFI verifica estas firmas antes de permitir que el software se ejecute.
- **Cadena de confianza**: Solo se permite ejecutar software que esté firmado con una clave que el firmware reconoce como confiable. Si alguna pieza de software no está firmada o la firma no es válida, el sistema no la cargará y generalmente no arrancará.

**Beneficios:**

- **Protección contra malware**: Al asegurarse de que solo se ejecute software de confianza durante el arranque, Secure Boot ayuda a proteger el sistema contra ciertos tipos de malware que se cargan antes del sistema operativo.
- **Integridad del sistema**: Mantiene la integridad del sistema al evitar modificaciones no autorizadas de componentes críticos del arranque.

### 2- BlackLotus

BlackLotus es un Bootkit UEFI diseñado específicamente para Windows. Incorpora un bypass de la protección Secure Boot y protección en Ring0 para evitar cualquier intento de eliminación. Una vez desplegado, un antivirus tradicional no podrá escanearlo ni eliminarlo.

**Características generales:**

- Escrito en C y x86asm.
- Utiliza las API de Windows NTAPI EFIAPI.
- El binario compilado tiene solo 80 KB de tamaño.
- Utiliza HTTPS para la comunicación con el C2.
- Bypass HVCI (Memory integrity and virtualization-based security).
- Bypass UAC (Control de cuentas de usuario).
- Bypass Secure Boot.
- Bypass de la secuencia de arranque de BitLocker.
- Bypass de Windows Defender.
- Ofrece un motor de Hooking de API.
- Motor Anti-Hooking: Para deshabilitar, eludir y controlar EDRs.

Este malware apareció por primera vez a la venta en foros de hacking en octubre de 2022 y es el primero que consigue saltarse la protección Secure Boot, por lo que puede usarse incluso en equipos actualizados. Para ello hace uso del CVE-2022-21894 (llamado baton drop) que, a pesar de ser parcheado por Windows en enero de 2022, puede continuar siendo explotado ya que muchos de los binarios vulnerables no han sido puestos en la “lista negra” de UEFI (UEFI Revocation List File) ya que hay muchísimos dispositivos sin actualizar que aún los usan.

Este bootkit contiene varias técnicas anti-VM, antidebugging y de ofuscación para que sea más difícil de replicar o analizar. Una vez instalado, el objetivo principal del bootkit es implementar un controlador de kernel y un downloader HTTP responsable de la comunicación con el C&C y capaz de cargar payloads adicionales en modo usuario o modo kernel.

**Curiosidades:**

Algunos de los instaladores de BlackLotus no continúan con la instalación del bootkit si el host comprometido usa una de las siguientes configuraciones regionales:

- Rumano (Moldavia) ro-MD
- Ruso (Moldavia) ru-MD
- Ruso (Rusia) ru-RU
- Ucraniano (Ucrania) uk-UA
- Bielorruso (Bielorrusia) be-BY
- Armenio (Armenia) hy-AM
- Kazajo (Kazajstán) kk-KZ

El código está lleno de referencias a la serie de anime Higurashi When They Cry, por ejemplo en nombres de componentes individuales como `higurashi_installer_uac_module.dll` y `higurashi_kernel.sys`.

El código descifra pero nunca usa varias strings que contienen mensajes del autor de BlackLotus (hasherezade es una investigadora conocida y autora de varias herramientas de análisis de malware) o simplemente algunas citas aleatorias de varias canciones, juegos o series.

### 3- Explotación de la vulnerabilidad

La explotación de esta vulnerabilidad permite la ejecución de código arbitrario en una etapa temprana del arranque del sistema cuando la aplicación aún es manejada por UEFI. Esto permite al atacante modificar cosas que no debería poder en un sistema con Secure Boot activado.

Como hemos comentado anteriormente, a pesar de que la vulnerabilidad ha sido parcheada, aún es posible su explotación ya que muchos binarios firmados siguen estando disponibles y el atacante puede instalar una versión vulnerable y legítima en el sistema para explotar la vulnerabilidad. Las aplicaciones .efi vulnerables permiten borrar de la memoria las políticas (policy) que usará el Secure Boot antes de haberse cargado usando un BCD manipulado. Esto permite usar opciones como “bootdebug” o “testsigning” que desactivan la protección Secure Boot.

La forma en que BlackLotus explota esta vulnerabilidad es la siguiente:

Después de haber instalado en el ESP los ficheros maliciosos, tras el primer reinicio se ejecutará un `bootmgfw.efi` modificado que usará una configuración BCD maliciosa que a su vez cargará un `hvloader.efi` con una configuración BCD que usará la opción “nointegritychecks” y “testsigning” (modos de “debug” que ofrece el SO para los desarrolladores) y que permite ejecutar aplicaciones no firmadas y, por tanto, bypasear esta protección. Tras esto, el malware gana consistencia para ejecutarse en cada arranque agregando su propia firma digital (MOK) en una zona de memoria que solo está disponible durante el arranque, pero tras la explotación de la vulnerabilidad tenemos acceso a dicha zona, por lo que una vez agregada la firma a la base de datos de MOK (moklist), el resto de las veces el bootkit se ejecutará como una aplicación lícita en el sistema, siendo prácticamente indetectable por el sistema.

![MOK boot process overview](https://github.com/jmnava/mrev/assets/6135174/01450482-b585-44f4-a9a4-0a40bb9dda7a)

### 4- Conclusión

Las vulnerabilidades durante el arranque del sistema permiten a un atacante el control total de un dispositivo ya que durante esta fase muchas de las protecciones implementadas aún no son efectivas y, además, a pesar de ser parcheadas de manera rápida como fue en este caso, no hace que se solucione el problema ya que, debido a la complejidad del ecosistema UEFI, una actuación drástica puede dejar inservible multitud de equipos, por lo que las vulnerabilidades continúan siendo efectivas durante mucho tiempo tras el parche.

### 5- Anexo

El concepto Secure Boot no es algo único en Windows, es algo que se está implementando en múltiples sistemas, desde móviles hasta pequeños dispositivos electrónicos. En los siguientes enlaces podéis ver un POC del funcionamiento del Secure Boot en un microcontrolador RT1050 de NXP. En este tipo de dispositivos el funcionamiento no es tan complejo como en el entorno UEFI, pero el concepto es el mismo: solo permitir la ejecución de un firmware firmado por una entidad que el dispositivo reconozca como legítima.

- [Cómo implementar un bootloader seguro en un dispositivo embebido](https://dmed-software.com/how-to-implement-a-secure-bootloader-in-an-embedded-device/)
- [Caso de estudio sobre la implementación de un bootloader seguro](https://dmed-software.com/how-to-implement-a-secure-bootloader-in-an-embedded-device-case-study/)

### Referencias

- [WeLiveSecurity: Bootkit UEFI BlackLotus es una realidad](https://www.welivesecurity.com/la-es/2023/03/02/bootkit-uefi-blacklotus-es-una-realidad/)
- [GitHub: CVE-2022-21894](https://github.com/Wack0/CVE-2022-21894)
- [DarkRelay: Windows 11's Secure Boot defeated by BlackLotus Malware (CVE-2022-21894)](https://www.darkrelay.com/post/windows-11-s-secure-boot-defeated-by-blacklotus-malware-cve-2022-21894)
- [WeLiveSecurity: BlackLotus UEFI Bootkit Myth Confirmed](https://www.welivesecurity.com/2023/03/01/blacklotus-uefi-bootkit-myth-confirmed/)
- [Microsoft Security Response Center: CVE-2022-21894](https://msrc.microsoft.com/update-guide/en-US/advisory/CVE-2022-21894)
- [GitHub: BlackLotusImage](https://github.com/ldpreload/)
