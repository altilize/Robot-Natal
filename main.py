import serial
import pygame
import time
import os

# Ganti baudrate sesuai dengan yang diubah di ESP32 Slave tadi
SERIAL_PORT = '/dev/ttyUSB0'
BAUD_RATE = 500000  # Harus sama dengan Serial.begin di Slave

SOUND_MAPPING = {
    'a': 'Sound/bioskop.mp3',
    'b': 'Sound/Jilat.mp3',
    'c': 'Sound/Natal.mp3',
    'd': 'Sound/Welcome.mp3'
}

pygame.mixer.init()

try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1) # Timeout kecil
    print(f"--- TERHUBUNG KE {SERIAL_PORT} (Low Latency Mode) ---")
    
    while True:
        # Trik Anti-Delay:
        # Baca semua data yang ada di buffer sampai habis, ambil yang terakhir.
        # Ini mencegah Python membaca data "masa lalu".
        if ser.in_waiting > 0:
            lines = ser.read(ser.in_waiting).split(b'\n')
            
            # Iterasi semua baris yang baru saja dibaca
            for line_bytes in lines:
                try:
                    line = line_bytes.decode('utf-8', errors='ignore').strip()
                    if not line: continue
                    
                    parts = line.split(' ', 1)
                    flag = parts[0]

                    # --- LOGIKA TOMBOL (Prioritas) ---
                    if flag == 'D' and len(parts) > 1:
                        cmd = parts[1]
                        print(f"[EVENT] Tombol Diterima: {cmd}")
                        if cmd in SOUND_MAPPING:
                            path = SOUND_MAPPING[cmd]
                            if os.path.exists(path):
                                if pygame.mixer.music.get_busy():
                                    pygame.mixer.music.stop()
                                pygame.mixer.music.load(path)
                                pygame.mixer.music.play()
                    
                    # --- LOGIKA MOTION (Hanya print yang terakhir untuk debug) ---
                    elif flag == 'M':
                        # Kita tidak perlu memproses semua M history, 
                        # tapi kalau mau debug, print saja.
                        pass # Biar terminal tidak penuh, atau print(line) jika perlu

                except Exception:
                    pass
            
            # Opsional: Jika hanya ingin melihat data M terakhir (Realtime display)
            # print(f"Last Data: {lines[-2] if len(lines) > 1 else lines[0]}")

        time.sleep(0.001) # Sleep sangat kecil agar CPU tidak 100%

except KeyboardInterrupt:
    if 'ser' in locals() and ser.is_open: ser.close()