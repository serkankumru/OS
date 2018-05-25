#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

// Semaforlar tanimlandi
sem_t berberler;                
sem_t musteriler;               
sem_t mutex;  

// Degiskenlerkenler  tanimlandi
int koltukS = 0;
int musteriS = 0;
int sandalyeS = 0;
int bosSandalyeS = 0;
int simdikiMusteri = 0;
int sandalye = 0;
int *koltuk;

void Berber (void *kimlik) {
    int sayi = *(int*)kimlik + 1;   // Berber kimligini alir.
    int mID, sonrakiMusteri;        

    printf("%d. Berber\t-->\tdukkana geldi.\n", sayi);

    while (1) {		//sonsuz dongu olusturuldu.
        if (!mID)	//musteru yoksa berber uyutulur
            printf("%d. Berber\t-->\tuyudu.\n\n", sayi);

        sem_wait(&berberler);   // Erisimi kilitleyip genel kilidi kaldiriyoruz.
        sem_wait(&mutex);       

        // Bekleyenlerin arasindan musteri sec
        simdikiMusteri = (++simdikiMusteri) % sandalyeS;
        sonrakiMusteri = simdikiMusteri;
        mID = koltuk[sonrakiMusteri];
        koltuk[sonrakiMusteri] = pthread_self();

        sem_post(&mutex);       // Eririmleri kaldir
        sem_post(&musteriler);  

        printf("%d. Berber\t-->\t%d. musterinin sacini kesmeye basladi.\n\n", sayi, mID);
        sleep(1);
        printf("%d. Berber\t-->\t%d. musterinin sacini kesmeyi bitirdi.\n\n", sayi, mID);
        
        if (sandalyeS == bosSandalyeS) {    // Bekleyen musteri yoksa uyu
            printf("%d. Berber\t-->\tuyudu.\n\n", sayi);
        }
    }
    
    pthread_exit(0);
}

void Musteri (void *kimlik) {
    int sayi = *(int*)kimlik + 1;// mustrinin kimligi
    int oturulanSandalye, bID;

    sem_wait(&mutex);   // Genel eririmi kilitle

    printf("%d. Musteri\t-->\tdukkana geldi.\n", sayi);

    // Bekleme odasinda bos sandalye varsa
    if (bosSandalyeS > 0) {
        bosSandalyeS--;

        printf("%d. Musteri\t-->\tbekleme salonunda bekliyor.\n\n", sayi);

        // Bekleme salonundan bir sandalye secip otur
        sandalye = (++sandalye) % sandalyeS;
        oturulanSandalye = sandalye;
        koltuk[oturulanSandalye] = sayi;

        sem_post(&mutex);           // Erisim kilidini kaldir
        sem_post(&berberler);       // Siradaki uygun berberi uyandir

        sem_wait(&musteriler);      // Bekleyen musteriler kuyruguna katil
        sem_wait(&mutex);           // Koltuga erisimi kilitle 

        // Berber koltuguna gec 
        bID = koltuk[oturulanSandalye];
        bosSandalyeS++;

        sem_post(&mutex);
    }
    else {      // Bekleme salonunda bos sandalye yoksa dukandan cik
        sem_post(&mutex);
        printf("%d. Musteri\t-->\tbekleme salonunda yer bulamadi. Dukkandan ayriliyor.\n\n", sayi);
    }
    pthread_exit(0);
}

int main (int argc) {
    // Gerekli verileri kullanicidan al
    printf("Musteri sayisini girin: ");
    scanf("%d", &musteriS);

    printf("Bekleme salonundaki sandalye sayisini girin: ");
    scanf("%d", &sandalyeS);

    printf("Berber koltugu sayisini girin: ");
    scanf("%d", &koltukS);

    bosSandalyeS = sandalyeS;                           // Bos sandalyeleri belirle
    koltuk = (int*) malloc(sizeof(int) * sandalyeS);    // Koltuk dizisini olustur.

    int berberKimlikleri[koltukS];

    pthread_t berber[koltukS];      // Berber threadlari
    pthread_t musteri[musteriS];    // Musteri threadlari

    // Semaforlar baslat
    sem_init(&berberler, 0, 0);
    sem_init(&musteriler, 0, 0);
    sem_init(&mutex, 0, 1);

    printf("\nBerber dukkani acildi.\n\n");
    int i;
    // Berber threadlari olustur
    for (i = 0; i < koltukS; i++) {
        pthread_create(&berber[i], NULL, (void*)Berber, (void*)&i);
        sleep(1);   // thread olusmasi icin 1 sn
    }

    // Musteri ipliklerini olustur
    for (i = 0; i < musteriS; i++) {
        pthread_create(&musteri[i], NULL, (void*)Musteri, (void*)&i);
        srand((unsigned int)time(NULL));
        usleep(rand() % (250000 - 50000 + 1) + 50000); // 50000 - 250000 ms
    }

    // Tum mmuserilerin islerinin bitmesini bekle
    for (i = 0; i < musteriS; i++) {
        pthread_join(musteri[i], NULL);
    }

    sleep(2);

    // Semaforlari yok et
    sem_destroy(&berberler);
    sem_destroy(&musteriler);
    sem_destroy(&mutex);

    printf("Musterilere hizmet verildi. Dukkan kapatilacak...\n\n");
    
    return 0;
}
