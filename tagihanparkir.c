#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_KENDARAAN 100
#define FILENAME "parkir_data.txt"

// Struktur Data Kendaraan
typedef struct {
    char plat[15];
    char jenis[10]; // "Mobil" atau "Motor"
    char waktu_masuk[25]; // Format: YYYY-MM-DD HH:MM:SS
    char waktu_keluar[25];
    long biaya;
    int status; // 1 = Parkir (Aktif), 0 = Keluar
} Kendaraan;

Kendaraan daftar_parkir[MAX_KENDARAAN];
int jumlah_kendaraan = 0;

// Prototype Fungsi
void muat_dari_file();
void simpan_ke_file();
long hitung_biaya(const char *jenis, int durasi_jam);
void tambah_kendaraan();
void keluar_kendaraan();
void cari_kendaraan();
void sorting_kendaraan();
void statistik_harian();
void tampilkan_masih_parkir();
void hapus_newline(char *str);
int cek_kesamaan_string(const char *s1, const char *s2);

// Fungsi pembantu untuk menghapus karakter newline dari fgets
void hapus_newline(char *str) {
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

// Fungsi bantu untuk membandingkan string tanpa peduli huruf besar/kecil (Case-Insensitive)
int cek_kesamaan_string(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
            return 0; // Tidak sama
        }
        s1++;
        s2++;
    }
    return (*s1 == *s2);
}

// 2. Hitung biaya parkir berdasarkan durasi (Fungsi dengan parameter)
long hitung_biaya(const char *jenis, int durasi_jam) {
    long tarif_per_jam = 3000; // Default
    
    if (cek_kesamaan_string(jenis, "Motor")) {
        tarif_per_jam = 2000;
    } else if (cek_kesamaan_string(jenis, "Mobil")) {
        tarif_per_jam = 5000;
    }
    
    if (durasi_jam < 1) durasi_jam = 1; // Minimal 1 jam
    return durasi_jam * tarif_per_jam;
}

// 1. Tambah Data Kendaraan dengan Validasi Jenis Kendaraan
void tambah_kendaraan() {
    if (jumlah_kendaraan >= MAX_KENDARAAN) {
        printf("\n[ERROR] Kapasitas parkir penuh!\n");
        return;
    }

    printf("\n--- TAMBAH KENDARAAN MASUK ---\n");
    char plat_input[15];
    printf("Masukkan Plat Nomor: ");
    
    // Membersihkan buffer input sebelum membaca string baru
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    if (fgets(plat_input, sizeof(plat_input), stdin) == NULL) return;
    hapus_newline(plat_input);

    // Ubah plat ke huruf kapital
    for (int i = 0; plat_input[i]; i++) {
        plat_input[i] = toupper((unsigned char)plat_input[i]);
    }

    // Cek apakah kendaraan dengan plat tersebut sedang parkir
    for (int i = 0; i < jumlah_kendaraan; i++) {
        if (strcmp(daftar_parkir[i].plat, plat_input) == 0 && daftar_parkir[i].status == 1) {
            printf("[WARNING] Kendaraan dengan plat %s masih tercatat parkir!\n", plat_input);
            return;
        }
    }

    char jenis_input[10];
    printf("Masukkan Jenis Kendaraan (Mobil/Motor): ");
    if (fgets(jenis_input, sizeof(jenis_input), stdin) == NULL) return;
    hapus_newline(jenis_input);

    // VALIDASI: Cek apakah input jenis kendaraan adalah "Mobil" atau "Motor"
    if (!cek_kesamaan_string(jenis_input, "Mobil") && !cek_kesamaan_string(jenis_input, "Motor")) {
        printf("[GAGAL] Jenis kendaraan tidak valid! Hanya boleh 'Mobil' atau 'Motor'. Data gagal dimasukkan.\n");
        return; // Keluar dari fungsi tanpa menyimpan data
    }

    Kendaraan k;
    strcpy(k.plat, plat_input);
    
    // Normalisasi format huruf kapital awal
    if (cek_kesamaan_string(jenis_input, "Mobil")) {
        strcpy(k.jenis, "Mobil");
    } else {
        strcpy(k.jenis, "Motor");
    }

    // Dapatkan waktu saat ini
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(k.waktu_masuk, sizeof(k.waktu_masuk), "%Y-%m-%d %H:%M:%S", t);

    strcpy(k.waktu_keluar, "-");
    k.biaya = 0;
    k.status = 1; // Aktif Parkir

    daftar_parkir[jumlah_kendaraan++] = k;
    simpan_ke_file();

    printf("[SUKSES] Kendaraan %s (%s) berhasil dicatat pada %s\n", k.plat, k.jenis, k.waktu_masuk);
}

// 1. & 2. Hapus/Keluar Kendaraan & Hitung Biaya
void keluar_kendaraan() {
    printf("\n--- PROSES KELUAR KENDARAAN ---\n");
    tampilkan_masih_parkir();

    char plat_input[15];
    printf("\nMasukkan Plat Nomor yang akan keluar: ");
    
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    if (fgets(plat_input, sizeof(plat_input), stdin) == NULL) return;
    hapus_newline(plat_input);

    for (int i = 0; plat_input[i]; i++) {
        plat_input[i] = toupper((unsigned char)plat_input[i]);
    }

    int idx = -1;
    for (int i = 0; i < jumlah_kendaraan; i++) {
        if (strcmp(daftar_parkir[i].plat, plat_input) == 0 && daftar_parkir[i].status == 1) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        printf("[ERROR] Kendaraan tidak ditemukan atau sudah keluar!\n");
        return;
    }

    // Waktu keluar saat ini
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(daftar_parkir[idx].waktu_keluar, sizeof(daftar_parkir[idx].waktu_keluar), "%Y-%m-%d %H:%M:%S", t);

    // Simulasi durasi parkir (untuk testing default 1 jam)
    int durasi_jam = 1; 

    daftar_parkir[idx].biaya = hitung_biaya(daftar_parkir[idx].jenis, durasi_jam);
    daftar_parkir[idx].status = 0; // Keluar

    simpan_ke_file();

    printf("\n--- STRUK PARKIR ---\n");
    printf("Plat Nomor   : %s\n", daftar_parkir[idx].plat);
    printf("Jenis        : %s\n", daftar_parkir[idx].jenis);
    printf("Waktu Masuk  : %s\n", daftar_parkir[idx].waktu_masuk);
    printf("Waktu Keluar : %s\n", daftar_parkir[idx].waktu_keluar);
    printf("Durasi       : %d jam\n", durasi_jam);
    printf("Total Biaya  : Rp %ld\n", daftar_parkir[idx].biaya);
}

// 7. Tampilkan Daftar Kendaraan yang Masih Parkir
void tampilkan_masih_parkir() {
    printf("\n--- DAFTAR KENDARAAN MASIH PARKIR ---\n");
    printf("%-5s | %-12s | %-10s | %-20s\n", "No", "Plat Nomor", "Jenis", "Waktu Masuk");
    printf("-----------------------------------------------------\n");

    int count = 0;
    for (int i = 0; i < jumlah_kendaraan; i++) {
        if (daftar_parkir[i].status == 1) {
            count++;
            printf("%-5d | %-12s | %-10s | %-20s\n",
                   count, daftar_parkir[i].plat, daftar_parkir[i].jenis, daftar_parkir[i].waktu_masuk);
        }
    }

    if (count == 0) {
        printf("Tidak ada kendaraan yang sedang parkir.\n");
    }
}

// 3. Cari Kendaraan Berdasarkan Plat Nomor
void cari_kendaraan() {
    printf("\n--- CARI KENDARAAN ---\n");
    char keyword[15];
    printf("Masukkan Plat Nomor yang dicari: ");
    
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    if (fgets(keyword, sizeof(keyword), stdin) == NULL) return;
    hapus_newline(keyword);

    for (int i = 0; keyword[i]; i++) {
        keyword[i] = toupper((unsigned char)keyword[i]);
    }

    printf("\n%-12s | %-10s | %-20s | %-8s | %-10s\n", "Plat Nomor", "Jenis", "Waktu Masuk", "Status", "Biaya");
    printf("-------------------------------------------------------------------\n");

    int ditemu = 0;
    for (int i = 0; i < jumlah_kendaraan; i++) {
        if (strstr(daftar_parkir[i].plat, keyword) != NULL) {
            ditemu++;
            char status_str[10];
            char biaya_str[15];

            if (daftar_parkir[i].status == 1) {
                strcpy(status_str, "Parkir");
                strcpy(biaya_str, "-");
            } else {
                strcpy(status_str, "Keluar");
                sprintf(biaya_str, "Rp %ld", daftar_parkir[i].biaya);
            }

            printf("%-12s | %-10s | %-20s | %-8s | %-10s\n",
                   daftar_parkir[i].plat, daftar_parkir[i].jenis, daftar_parkir[i].waktu_masuk, status_str, biaya_str);
        }
    }

    if (ditemu == 0) {
        printf("Kendaraan dengan plat '%s' tidak ditemukan.\n", keyword);
    }
}

// 4. Sorting Kendaraan Berdasarkan Waktu Masuk atau Jenis
void sorting_kendaraan() {
    if (jumlah_kendaraan <= 1) {
        printf("\nData kurang untuk di-sorting.\n");
        return;
    }

    printf("\n--- SORTING KENDARAAN ---\n");
    printf("1. Berdasarkan Waktu Masuk\n");
    printf("2. Berdasarkan Jenis Kendaraan\n");
    printf("Pilih (1/2): ");
    int pil;
    scanf("%d", &pil);

    // Bubble Sort
    for (int i = 0; i < jumlah_kendaraan - 1; i++) {
        for (int j = 0; j < jumlah_kendaraan - i - 1; j++) {
            int tukar = 0;
            if (pil == 1) { // Berdasarkan Waktu Masuk
                if (strcmp(daftar_parkir[j].waktu_masuk, daftar_parkir[j + 1].waktu_masuk) > 0) tukar = 1;
            } else if (pil == 2) { // Berdasarkan Jenis
                if (strcmp(daftar_parkir[j].jenis, daftar_parkir[j + 1].jenis) > 0) tukar = 1;
            }

            if (tukar) {
                Kendaraan temp = daftar_parkir[j];
                daftar_parkir[j] = daftar_parkir[j + 1];
                daftar_parkir[j + 1] = temp;
            }
        }
    }

    simpan_ke_file();
    printf("[SUKSES] Data berhasil di-sorting!\n");
    tampilkan_masih_parkir();
}

// 6. Statistik Harian
void statistik_harian() {
    printf("\n--- STATISTIK HARIAN PARKIR ---\n");
    int total = jumlah_kendaraan;
    int sedang_parkir = 0;
    int sudah_keluar = 0;
    long total_pendapatan = 0;

    for (int i = 0; i < jumlah_kendaraan; i++) {
        if (daftar_parkir[i].status == 1) {
            sedang_parkir++;
        } else {
            sudah_keluar++;
            total_pendapatan += daftar_parkir[i].biaya;
        }
    }

    printf("Total Kendaraan Tercatat : %d\n", total);
    printf("Sedang Parkir (Di Dalam) : %d\n", sedang_parkir);
    printf("Sudah Keluar             : %d\n", sudah_keluar);
    printf("Total Pendapatan Harian  : Rp %ld\n", total_pendapatan);
}

// 5. Simpan Data ke File Teks
void simpan_ke_file() {
    FILE *f = fopen(FILENAME, "w");
    if (f == NULL) {
        printf("[ERROR] Gagal menyimpan ke file!\n");
        return;
    }

    for (int i = 0; i < jumlah_kendaraan; i++) {
        fprintf(f, "%s|%s|%s|%s|%ld|%d\n",
                daftar_parkir[i].plat,
                daftar_parkir[i].jenis,
                daftar_parkir[i].waktu_masuk,
                daftar_parkir[i].waktu_keluar,
                daftar_parkir[i].biaya,
                daftar_parkir[i].status);
    }
    fclose(f);
}

// 5. Muat Data dari File Teks
void muat_dari_file() {
    FILE *f = fopen(FILENAME, "r");
    if (f == NULL) return; // File belum ada

    jumlah_kendaraan = 0;
    while (!feof(f) && jumlah_kendaraan < MAX_KENDARAAN) {
        Kendaraan k;
        if (fscanf(f, "%[^|]|%[^|]|%[^|]|%[^|]|%ld|%d\n",
                   k.plat, k.jenis, k.waktu_masuk, k.waktu_keluar, &k.biaya, &k.status) == 6) {
            daftar_parkir[jumlah_kendaraan++] = k;
        }
    }
    fclose(f);
}

// Main Menu
int main() {
    muat_dari_file();
    int pilihan;

    do {
        printf("\n==========================================\n");
        printf("      PARKING LOT MANAGEMENT SYSTEM       \n");
        printf("==========================================\n");
        printf("1. Tambah Kendaraan Masuk\n");
        printf("2. Proses Kendaraan Keluar & Hitung Biaya\n");
        printf("3. Cari Kendaraan Berdasarkan Plat\n");
        printf("4. Sorting Data Kendaraan\n");
        printf("5. Statistik Harian & Pendapatan\n");
        printf("6. Tampilkan Daftar Kendaraan Sedang Parkir\n");
        printf("7. Keluar Program\n");
        printf("Pilih menu (1-7): ");
        
        if (scanf("%d", &pilihan) != 1) {
            printf("\n[ERROR] Masukkan angka yang valid!\n");
            while (getchar() != '\n'); // Clear buffer input
            pilihan = 0;
        }

        switch (pilihan) {
            case 1: tambah_kendaraan(); break;
            case 2: keluar_kendaraan(); break;
            case 3: cari_kendaraan(); break;
            case 4: sorting_kendaraan(); break;
            case 5: statistik_harian(); break;
            case 6: tampilkan_masih_parkir(); break;
            case 7: printf("\nTerima kasih! Program selesai.\n"); break;
            default: 
                if (pilihan != 0) printf("\nPilihan tidak valid! Coba lagi.\n");
        }
    } while (pilihan != 7);

    return 0;
}
