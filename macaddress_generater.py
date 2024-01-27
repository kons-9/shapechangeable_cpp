import os

# mac address is written in data/macaddress.txt like 0000000f(uint32)
# nodeid:29:locallocation:2:is_root:1

def parse_macaddress(macaddress: str):
    

    nodeid = macaddress[0:29]
    locallocation = macaddress[29:31]
    is_root = macaddress[31]
    return nodeid, locallocation, is_root

def generate_macaddress(nodeid: str, locallocation: str, is_root: str):
    macaddress = nodeid + locallocation + is_root
    return macaddress

def get_macaddress():
    with open('data/macaddress.txt', 'r') as f:
        macaddress = f.read()
    # convert hex to binary
    try:
        macaddress  = bin(int(macaddress, 16))[2:].zfill(32)
    except:
        macaddress = generate_macaddress('0'*29, '00', '1')
    return macaddress

def is_exist_file(path: str) -> bool:
    if os.path.isfile(path):
        return True
    else:
        return False
    
def validate(macaddress: str) -> bool:
    nodeid, locallocation, is_root = parse_macaddress(macaddress)
    # root's nodeid is 0x00000000
    if is_root == '1' and nodeid != '0'*29:
        return False
    # if not root, nodeid is not 0x00000000
    if is_root == '0' and nodeid == '0'*29:
        return False
    
    # if macaddress is in hist/used_macaddress.txt, return False
    with open('hist/used_macaddress.txt', 'r') as f:
        used_macaddress = f.read()
    if macaddress in used_macaddress:
        return False
    return True

def update_macaddress(macaddress: str):
    # recreate macaddress.txt
    with open('data/macaddress.txt', 'w') as f:
        hex_macaddress = hex(int(macaddress, 2))[2:].zfill(8)
        f.write(hex_macaddress)
    # append in used_macaddress.txt
    with open('hist/used_macaddress.txt', 'a') as f:
        f.write(macaddress + '\n')
        
def next_macaddress(macaddress: str) -> str:
    nodeid, locallocation, is_root = parse_macaddress(macaddress)
    match locallocation:
        case '00':
            locallocation = '01'
        case '01':
            locallocation = '10'
        case '10':
            locallocation = '11'
        case '11':
            locallocation = '00'
            next_nodeid = int(nodeid, 2) + 1
            nodeid = bin(next_nodeid)[2:].zfill(29)
            is_root = '0'
    macaddress = generate_macaddress(nodeid, locallocation, is_root)
    return macaddress

def print_macaddress(macaddress: str):
    nodeid, locallocation, is_root = parse_macaddress(macaddress)
    # 2 -> 10
    location_str = {
        '00': 'upper left',
        '01': 'upper right',
        '10': 'lower left',
        '11': 'lower right'
    }
    print('nodeid: ', int(nodeid, 2))
    print('locallocation: ', location_str[locallocation])
    print('is_root: ', bool(int(is_root)))

if __name__ == '__main__':
    if not is_exist_file('hist/used_macaddress.txt'):
        raise FileNotFoundError("hist/used_macaddress.txt is not exist")

    if is_exist_file('data/macaddress.txt'):
        macaddress = get_macaddress()
    else:
        macaddress = generate_macaddress('0'*29, '00', '1')

    while not validate(macaddress):
        macaddress = next_macaddress(macaddress)
    update_macaddress(macaddress)
    print_macaddress(macaddress)