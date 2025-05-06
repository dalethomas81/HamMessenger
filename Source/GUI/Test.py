# pip install aprspy

from aprspy import APRS

# Example MIC-E encoded message
mic_e_messages = ['KN4UAH-7>SWSRYY,WA6TOW-2,WIDE1*,WIDE2-1,qAR,W6SRR-3:`1TCmi7[/`"49}_3',
                  'YO8RXT-OG>APOBJ,TCPIP*,qAU,T2ROMANIA:;145.2875L*111111z4709.78N/02736.33Er145.287MHz T103 R20k teste RoLink-net.ro/A=00032',
                  'IQ3AZ-11>APNL51,TCPIP*,qAC,OE2XZR-10:!4540.83N/01323.68E&Igate Nanopi in FK105 udpgate 0.77',
                  'NI4CE-10>APMI06,TCPIP*,qAC,T2PANAMA:@060623z2724.49N/08214.96W_032/000g000t064r000p000P000h00b10149U2k']

for message in mic_e_messages:

    packet = APRS.parse(message)

    try:
        print(packet)
        print(packet.latitude)
        print(packet.longitude)
        print(packet.course)
        
    except:
        pass

