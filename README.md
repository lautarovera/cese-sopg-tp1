# CESE - SOPG TP 1

## Introducción

El proyecto consta de dos programas, *Reader* y *Writer*, que se comunican mediante una FIFO nombrada.  
El Writer escribe mensajes sobre la FIFO, y el Reader los lee.  
El Reader filtra los mensajes, admitiendo sólo mensajes con el prefijo "DATA:", descartando el prefijo y colocando los mensajes línea a línea en el archivo "Log.txt".  
El Writer admite las señales de usuario SIGUSR1 y SIGUSR2. Al recibir estas señales envía de manera automática los mensajes "SIGN:1" y "SIGN:2" respectivamente. En este caso, el Reader colocará este tipo de mensajes con prefijo "SIGN:" en el archivo "Sign.txt".

## Compilación

Suponiendo que el repositorio fue clonado en '~', los siguientes comandos ejecutados desde la raíz compilaran los programas Reader y Writer:  

```console
username@host:~/cese-sopg-tp1$ gcc src/reader.c -o objs/Reader
username@host:~/cese-sopg-tp1$ gcc src/writer.c -o objs/Writer
```

## Ejecución

### Reader

Desde la raíz, abrir un terminal y ejecutar lo siguiente:  

```console
username@host:~/cese-sopg-tp1$ objs/Reader
```

### Writer

También desde raíz, abrir otro terminal distinto al anterior y ejecutar lo siguiente:  

```console
username@host:~/cese-sopg-tp1$ objs/Writer
```
