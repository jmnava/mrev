# Pasos para Elevar Privilegios con WinDbg

## Instalación y Preparación

1. Instalar WinDbg y activar el modo test en la máquina virtual.

![image](https://github.com/jmnava/mrev/assets/6135174/d247e2e3-0fbe-4421-85f3-f2b593aca564)


## Uso de WinDbg

1. Abrir WinDbg y conectar el debugger a la máquina.

![image](https://github.com/jmnava/mrev/assets/6135174/48cbf3d3-f3af-446d-a585-bb7969cf123b)


2. Abrir un cmd y ejecutar el siguiente comando para verificar el acceso a las estructuras del proceso cmd.exe:

```!process 0 0 cmd.exe```

![image](https://github.com/jmnava/mrev/assets/6135174/ea8361e7-4947-4e21-b57e-640236a67107)

Obtener la dirección del proceso y acceder a los campos de la estructura `EPROCESS` con:

```dt !_EPROCESS ffffe20f9db02340```

![image](https://github.com/jmnava/mrev/assets/6135174/d4649774-132e-49fa-97ce-a459f217c7de)

Vemos que efectivamente se trata del proceso cmd.exe

![image](https://github.com/jmnava/mrev/assets/6135174/5db4c86d-a5a5-489f-95e5-de8551db2961)


## Elevación de Privilegios

1. Verificar los privilegios del proceso cmd.exe:
   
![image](https://github.com/jmnava/mrev/assets/6135174/07b2ee9e-db81-47bb-befb-c741b5564fdc)



Los privilegios están en el campo `token`, que es una estructura `EX_FAST_REG` dentro de `EPROCESS`, ubicada en el offset 4B8.

![image](https://github.com/jmnava/mrev/assets/6135174/c48c6625-d954-4333-8cb5-7b8440dc8d55)


3. Ver el contenido de la estructura:

```dt !_EX_FAST_REF ffffe20f9db02340+4b8```

![image](https://github.com/jmnava/mrev/assets/6135174/03504fb3-cb16-4ed8-8168-279a6611ea05)

El contenido es un puntero a la memoria que contiene los permisos; quitar el último byte que hace referencia al contador y que podemos consultar con:

```dt !_TOKEN 0xffff8d89`5b91c060```

![image](https://github.com/jmnava/mrev/assets/6135174/297f37b3-8fa8-458c-9bbe-c77276bd27be)


Estos permisos están en una estructura Privileges.

![image](https://github.com/jmnava/mrev/assets/6135174/ef168019-b311-42e7-9160-c66b166dc94a)

3. Verificar los permisos específicos con:

```dt !_SEP_TOKEN_PRIVILEGES 0xffff8d89`5b91c060+0x40```

![image](https://github.com/jmnava/mrev/assets/6135174/bd6af41c-0cd1-43de-99a2-24b463cb4553)


4. También podemos usar el comando !token que nos ahorra varios pasos:

```!token 0xffff8d89`5b91c060```

![image](https://github.com/jmnava/mrev/assets/6135174/1c1de99f-3b3f-44d1-8b83-3600a4cfdf27)


5. Si ahora queremos modificar dichos privilegios usaremos el comando `eq`
   
```eq 0xffff8d89`5b91c060+0x40+0x008 0x0000000602880000```

![image](https://github.com/jmnava/mrev/assets/6135174/7a43b454-975e-4d21-849f-b5da3339548d)


Y vemos que nuestro cmd ha aumentado sus privilegios

![image](https://github.com/jmnava/mrev/assets/6135174/9e94e43d-da0e-45d1-b240-93f364bee5fb)


## Automatización del Proceso

Para automatizar estos pasos, hemos creado un script en Python que permite ingresar el nombre del proceso para elevar sus privilegios. Puedes encontrar el script en el siguiente enlace:

[https://github.com/jmnava/mrev/blob/main/ElevatePrivileges.py](https://github.com/jmnava/mrev/blob/main/ElevatePrivileges.py)

```pyton

import pykd

def find_process(process_name):
    try:
        # Use the WinDbg command to list all processes
        process_list = pykd.dbgCommand("!process 0 0")

        # Check if the process name is in the list
        if process_name.lower() in process_list.lower():
            print(f"Process '{process_name}' is running.")
            return True
        else:
            print("Process not found.")
            return False
    except Exception as e:
        print(f"Error retrieving process list: {e}")
        return False

def find_process_address(eprocess):
    #Look for the address that come after PROCESS
    process_info = eprocess.split()
    for i in range(len(process_info)):
            if process_info[i].lower() == "process":
                process_address = process_info[i + 1]
                print(f"Memory address for process is: {process_address}")
                return process_address
            print("Process not found.")
    return None


def find_token_address(token):
 # Find the position of the colon
        colon_pos = token.index(':')
        
        # Extract the address part after the colon
        memory_address = token[colon_pos + 1:].strip().split()[0]
        
        # Remove the backtick and combine the parts
        memory_address = memory_address.replace('`', '')

        # Convert the memory address to an integer
        memory_address_int = int(memory_address, 16)
        
        # Zero out the last 4bits by masking with 0xFFFFFFFFFFFFFFF0
        adjusted_memory_address_int = memory_address_int & 0xFFFFFFFFFFFFFFF0
        
        # Convert the adjusted memory address back to a hexadecimal string
        adjusted_memory_address = hex(adjusted_memory_address_int)

        print(f"find token in address: {adjusted_memory_address}")

        return adjusted_memory_address

def get_present_privileges(token_privileges):
        # Find the position of the first colon that is the present privileges
        colon_pos = token_privileges.index(':')
        
        # Extract the value part after the colon
        present_value = token_privileges[colon_pos + 1:].strip().split()[0]
        
        # Convert the extracted value to an integer
        present_privileges = int(present_value.replace('`', ''), 16)

        return present_value


#The offset of the token in my windows version is 0x4B8
TOKEN_OFFSET = 0x4b8

#The offset of the privileges in my windows version is 0x40
PRIVILEGES_OFFSET = 0x40

#The offset of the present values of privileges in my windows version is 0x08
ENABLED_OFFSET = 0x08

def main():
    # Print Title
    print("Token Abuse example")
    
    # Ask user the name of the process
    process = input("Please enter the name of the process you want to increase the privileges: ")

    if find_process(process):
        # If the process is running, continue
        print(f"Get process info for: {process}")
        
        # Execute the WinDbg command !process 0 0 and get the output
        windbg_command = f"!process 0 0 {process}"
        try:
            result = pykd.dbgCommand(windbg_command)
            #find process address
            process_address = find_process_address(result)
            if process_address:
                #get token and cast as EX_FAST_REF
                windbg_command = f"dt !_EX_FAST_REF {process_address}+{TOKEN_OFFSET:#x}"
                result = pykd.dbgCommand(windbg_command)

                #Find token adresss
                token_addr = find_token_address(result)

                #Get privileges from token structure
                windbg_command = f"dt !_SEP_TOKEN_PRIVILEGES {token_addr}+{PRIVILEGES_OFFSET:#x}"
                # print (windbg_command)
                result = pykd.dbgCommand(windbg_command)
                print(f"Process '{process}' initial privileges: \n {result}")

                #set Enabled privileges with the value from present
                present_priv = get_present_privileges(result)

                #Set privileges from token structure
                windbg_command = f"eq {token_addr}+{PRIVILEGES_OFFSET:#x}+{ENABLED_OFFSET:#x} {present_priv}"
                # print (windbg_command)
                result = pykd.dbgCommand(windbg_command)

                #Get privileges from token structure for check the result
                windbg_command = f"dt !_SEP_TOKEN_PRIVILEGES {token_addr}+{PRIVILEGES_OFFSET:#x}"
                # print (windbg_command)
                result = pykd.dbgCommand(windbg_command)

                print(f"Process '{process}' final privileges: \n {result}")

        except Exception as e:
            print(f"Error executing WinDbg command: {e}")

if __name__ == "__main__":
    main()
```
Y la salida obtenida es esta

``` 
lkd> !py C:\Users\Jose\Desktop\ABR_Debugging_Scripting\WinDbg_Python\TokenAbuse 
Token Abuse example
Please enter the name of the process you want to increase the privileges: mspaint.exe
Process 'mspaint.exe' is running.
Get process info for: mspaint.exe
Memory address for process is: ffff8c8885c2c340
find token in address: 0xffffc288f73cd060
Process 'mspaint.exe' initial privileges: 
 nt!_SEP_TOKEN_PRIVILEGES
   +0x000 Present          : 0x00000006`02880000
   +0x008 Enabled          : 0x800000
   +0x010 EnabledByDefault : 0x40800000

Process 'mspaint.exe' final privileges: 
 nt!_SEP_TOKEN_PRIVILEGES
   +0x000 Present          : 0x00000006`02880000
   +0x008 Enabled          : 0x00000006`02880000
   +0x010 EnabledByDefault : 0x40800000
``` 
o esto en caso de que el proceso no se encuentre en ejecución

```
lkd> !py C:\Users\Jose\Desktop\ABR_Debugging_Scripting\WinDbg_Python\TokenAbuse 
Token Abuse example
Please enter the name of the process you want to increase the privileges: mspaint.exe
Process not found.
```



