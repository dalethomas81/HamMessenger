# pip install aprspy

from aprspy import APRS
import re

# Example MIC-E encoded message
'''mic_e_messages = [
                    'KN4UAH-7>SWSRYY,WA6TOW-2,WIDE1*,WIDE2-1,qAR,W6SRR-3:`1TCmi7[/`"49}_3'
                  #,'YO8RXT-OG>APOBJ,TCPIP*,qAU,T2ROMANIA:;145.2875L*111111z4709.78N/02736.33Er145.287MHz T103 R20k teste RoLink-net.ro/A=00032'
                  #,'IQ3AZ-11>APNL51,TCPIP*,qAC,OE2XZR-10:!4540.83N/01323.68E&Igate Nanopi in FK105 udpgate 0.77'
                  #,'NI4CE-10>APMI06,TCPIP*,qAC,T2PANAMA:@060623z2724.49N/08214.96W_032/000g000t064r000p000P000h00b10149U2k'
                  ,'KN6ARG-9>SWQTWR,WIDE1-1:`2Z5lr|j/`"7I}146.520MHz_1'
                  #,'Raw Modem:SRC: [KN6ARG-9] DST: [SWQTVT-0] PATH: [BKELEY-0] [WIDE1-0] [FMTPK-0] DATA: `2Z.mR/j/`"74}146.520MHz_1'
                  ]

for message in mic_e_messages:

    packet = APRS.parse(message)

    try:
        print(packet)
        print(packet.latitude)
        print(packet.longitude)
        print(packet.course)

    except:
        pass'''

def parse_raw_modem_fields(line: str) -> dict:
    pattern = (
        r'Raw Modem:?\s*'                              # allow “Raw Modem:” or “Raw Modem:”
        r'SRC:\s*\[(?P<SRC>[^\]]+)\]\s*'               # SRC:[…]
        r'DST:\s*\[(?P<DST>[^\]]+)\]\s*'               # DST:[…]
        r'PATH:\s*(?P<PATH>(?:\[[^\]]+\]\s*)+)\s*'     # one or more PATH:[…] groups
        r'DATA:\s*(?P<DATA>.*)'                        # everything after DATA:
    )
    m = re.match(pattern, line, re.IGNORECASE)
    if not m:
        return {}
    fields = m.groupdict()
    fields['PATH'] = ",".join(re.findall(r'\[([^\]]+)\]', fields['PATH']))
    
    return fields


def decode_aprs(line):
    fields = parse_raw_modem_fields(line)
    if fields:
        # KN6ARG-9>SWQTWR,WIDE1-1:`2Z5lr|j/`"7I}146.520MHz_1
        message = fields['SRC'] + '>' + fields['DST'] + ',' + fields['PATH'] + ':' + fields['DATA']
        try:
            print(message)
            packet = APRS.parse(message)
            return packet
        except:
            pass

    return None


# Test line
# KN6ARG-9>SWQTWR,WIDE1-1:`2Z5lr|j/`"7I}146.520MHz_1
# KN4UAH-7>APNU19-0,WILLAM-0:`1TCmi7[/`"49}_3
print(decode_aprs('Raw Modem:SRC: [KN6ARG-9] DST: [SWQTWR] PATH: [WIDE1-1] DATA: `2Z5lr|j/`"7I}146.520MHz_1'))
print(decode_aprs('Raw Modem:SRC: [KN4UAH-7] DST: [APNU19-0] PATH: [WILLAM-0] DATA: `1TCmi7[/`"49}_3'))
print(decode_aprs('Raw Modem:SRC: [CHAIX-0] DST: [APOT30-0] PATH: [SUMMIT-0] [FMTPK-0] DATA: !3850.09N/12034.62W# 13.2V'))

print(decode_aprs("Raw Modem:SRC: [W6TST-1] DST: [S6TYPQ-0] PATH: [FMTPK-0] [WIDE2-1] DATA: '13)l -/]TST="))
print(decode_aprs('Raw Modem:SRC: [K6TS-0] DST: [3VTYTV-0] PATH: [K6LY-3] [WIDE1-0] [FMTPK-0] DATA: `1GMl 2[/>"41}='))
print(decode_aprs('Raw Modem:SRC: [WILLAM-0] DST: [APRS-0] PATH: [FMTPK-0] DATA: }DB1NTO-2>APPIC3,TCPIP,WILLAM*::KD7SWQ-7 :ack1'))
print(decode_aprs('Raw Modem:SRC: [WILLAM-0] DST: [APRS-0] PATH: [FMTPK-0] DATA: }DB1NTO-2>APPIC3,TCPIP,WILLAM*::KD7SWQ-7 :Firmware update 022 available! Look at www.db1nto.de{460'))
print(decode_aprs('Raw Modem:SRC: [AC6EE-3] DST: [APN391-0] PATH: [WILLAM-0] [FMTPK-0] DATA: !3509.83NN11834.81W#PHG5905/A=006020/Bear Valley Springs Digi'))
#print(decode_aprs("""Raw Modem:SRC: [N8MOR-7] DST: [DHEQUY-0] PATH: [W6SRR-3] [N6ZX-3] [WIDE2-0] DATA: `1HjlSLK\]"5W}="""))
print(decode_aprs('Raw Modem:SRC: [N8QH-9] DST: [APOT30-0] PATH: [N8QH-9] [N6ZX-3] [WIDE2-0] DATA: T#199,523,263,247,168,195,00000000'))


'''
import base64

with open("blank.png", "wb") as f:
    f.write(base64.b64decode(
        "iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/x8AAusB9YdDdVwAAAAASUVORK5CYII="
    ))
'''

'''from PIL import Image

img = Image.new("RGBA", (24, 24), (0, 0, 0, 0))  # Fully transparent
img.save("icons/blank.png")

try:
    img = Image.open("Source/GUI/icons/blank.png")
    img.verify()  # Checks integrity
    print("Image is valid")
except Exception as e:
    print(f"Invalid image: {e}")'''