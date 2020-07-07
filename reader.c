#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFSIZE 2048 // myData uygulmasından  gelen verileri atayacagımız degişken

// genel hata yazdırma ve programdan çıkma kütüphanemiz
#define err(mess)                            \
    {                                        \
        fprintf(stderr, "Error: %s.", mess); \
        exit(1);                             \
    }

int main()
{
    int fd[2];
    ssize_t n, t;       // verilen dogru bir şekild okunup yazıldıgını kontrol eden degişkenlerimiz
    char buf[BUFFSIZE]; // gelen verileri tutan arrayimiz
    char message[1];    // kullanıcının girdigi karakteri tutan degişkenimiz

    // eğer fifo2_x dosyası daha oluşturulmadıysa myData çalışmıyordur.
    // myMore kendi başına çalışamaz

    if (access("fifo2_x", F_OK) == -1)
    {
        printf("önce MyData uygulamasını Çalıştırın");
        exit(0);
    }

    // child processden oku
    if ((fd[0] = open("fifo2_x", O_RDONLY)) < 0)
        err("open")

    // child processe veri yollamak için fifo1_x dosyasını oluştur.
    mkfifo("fifo1_x", S_IRWXU);

    if ((fd[1] = open("fifo1_x", O_WRONLY)) < 0)
        err("open")

    while ((n = read(fd[0], buf, BUFFSIZE)) > 0)
        {

            // myData programından gelen dosya içeriklerini yazdır.
            if (write(STDOUT_FILENO, buf, n) != n)
            {
                exit(1);
            }
            
            // eğer dosya tamamen okundu ise programı sonlandır
            if (access("kontrol", F_OK) == -1)
            {
                exit(0);
            }

            // dosya tamamen bittmedi ise kullanıcıdan tercihini girmesini bekle
            if ((t = read(STDIN_FILENO, message, 1)) > 0)
            {
                // kullancı q veya space karakterlerinden birini girmez ise çık
                if (!((message[0] == 'q') || (message[0] == 'Q') || (message[0] == ' ')))
                {
                    err("hatali işlem");
                }

                // girilen karakteri myData programına yolla
                if (write(fd[1], message, t) != t)
                {
                    err("write");
                }

                // q harfi girilmiş is döngüyü kır ve kapan
                if (message[0] == 'q')
                {
                    break;
                }
            }
        }
    
    // pipeları kapat ve programı sonlandır.
    close(fd[0]);
    close(fd[1]);
    return 0;
}
