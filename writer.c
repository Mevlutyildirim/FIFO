
#include <stdio.h>	// standart input-output kütüphanesi
#include <stdlib.h> // standart genel kütüphanemiz
#include <fcntl.h>
#include <sys/types.h> // tanımlanmış semboller ve yapılar içerir
#include <sys/stat.h>
#include <string.h> // string ile ilgili işlemlerde kullanacagımız kütüphanemiz
#include <unistd.h> // unix işletim sitemlerinde kullanacagımız kütüphane

// genel program hatalarını yazdırmak ve programdan çıkmak için
#define err(mess)                        \
	{                                      \
		fprintf(stderr, "Error: %s.", mess); \
		exit(1);                             \
	}

int main(int argc, char *argv[])
{

	int fd[2]; // pipe oluşturmak için kullanacagımız array
	pid_t pid; // process tanımlamak için

	// dosya açma ve okuma işlemlerinde kullanacagımız değişkenlerimiz
	FILE *fp;				
	char *filename; 
	char *line;
	size_t len = 0;
	ssize_t readFile;

	char file_content[1000]; // myData process'inden dosya içeriklerini yollamak için kullanacagımız değişken
	char message[2048];	// myMore dosyasından gelen verileri tutacak degişkenimiz

	// Dosya isminin girildiginden emin olmak için
	if (argc < 2)
	{
		printf("Hatalı Giriş Lütfen Dosya adını giriniz\n");
		return (1); // burayı degiştir
	}

	// sadece dosya adı girildiyse myMore tek başına çalışacak
	if (argc == 2)
	{

		filename = argv[1]; // dosya ismini al

		fp = fopen(filename, "r"); // dosyayı aç

		// Dosya açılamazsa çıkış yap
		if (fp == NULL)
			err("dosya  açılamadı");

		while ((readFile = getline(&line, &len, fp)) != -1)
		{

			printf("%s", line); // getline fonksiyonu ile satır satır yazdır.
		}

		fclose(fp); // dosyayı kapat

		exit(0);
	}

	if ((argc == 4))
	{

		// Bu durumda myMore uygulamasına veri yollamamız lazım;

		// iki stringi karşılaştırmak için strcmp fonksiyonunu kullanıyoruz
		// girilen 2. argüman = degilse çık
		if (!(strcmp(argv[2], "=") == 0))
		{
			err("hatalı argüman");
		}

		// girilen 3. argüman myMore degilse çık
		if (!(strcmp(argv[3], "myMore") == 0))
		{
			err("hatalı argüman");
		}

		pid = fork(); // child process'i oluştur

		filename = argv[1];
		fp = fopen(filename, "r");

		// Dosya açılamazsa çıkış yap
		if (fp == NULL)
		{
			err("dosya açılamadı argüman");
			;
		}

    
		// sıradan ve isimli pipe olarak iki tane pipe vardır 
		// parent ve child prosesler için sıradan pipe tanımlayabilirdik 
		// isimli pipe daha esnek bir kullanım olanagı sağladıgı için onu tercih ettim

		mode_t theMode = S_IRWXU; // açılan fifo dosyaları için her türlü yetkiyi ver
		mkfifo("fifo2_x", S_IRWXU); // mydata-myMore uygulamaları arası iletişim için.
		mkfifo("kontrol", S_IRWXU); // mymore uygulamasına dosyanın bittigini haber vermek için

		int i = 0;	// dosyanın okunan satrı sayısını tutan değişkenimiz
		ssize_t t, k; // okuma ve yazma işlerini dogru oldugundan emin olmak için

		if (pid < 0)
		{ 
			err("fork başarısız oldu.");
		}

		if (pid > 0)
		{   
		    // parent prosesdeyiz 
			// mymore uygulamasına veri yollamak için fifolarımızı açıyoruz. 
			if ((fd[0] = open("fifo2_x", O_WRONLY)) < 0)
				err("fifo2_x açılamadı");

			if ((fd[1] = open("fifo1_x", O_RDONLY)) < 0)
				err("fifo1_x açılamadı");

			while ((readFile = getline(&line, &len, fp)) != -1)
			{

				strcat(file_content, line); // satırımız file content degişkenimize at 

				i++;

				if (i % 24 != 0)
				{
					continue; // eğer satır sayısı 24 veya onun katı degilse geri kalan adımları atla
				}
        
				//satır sayısını 24. tamamladıgımız zaman fif02_x dosyasına yazdır.
				t = strlen(file_content) + 1;
				if (write(fd[0], file_content, t) != t)
				{
					err("dosya yazılamadı"); // dosya dügün yazılmaz ise çıkış yap
				}

				memset(file_content, 0, sizeof(file_content)); // file_content degişkenimizi sıfırla

				if ((k = read(fd[1], message, 2048)) > 0) // child prosesden gelen verileri al
				{
					// kullanıcı q harfi girdiyse  bütün pipelerı kapat, fifoları sil ve çıkış yap
					if (message[0] == 'q')
					{
						fclose(fp); //dosyayımızı ve pipeları kapat. fifoları sil
						close(fd[0]);
						close(fd[1]);
						
						unlink("fifo2_x");
						exit(0);
					}
				}
			}
      
			// geriye kalan satırları yolla
			unlink("kontrol"); // mymore'a dosyanın bittigini haber vermek için bu fifoyu sil
			t = strlen(file_content) + 1;
			if (write(fd[0], file_content, t) != t)
			{
				err("dosya yazılamadı");
			}

			fclose(fp); //dosyayımızı ve pipeları kapat. fifoları sil
			close(fd[0]);
			close(fd[1]);
			unlink("fifo2_x");
			waitpid(pid, NULL, 0); // child prosesin bitmesini bekle
		}

		else
		{
         
        // isimli pipe kullandıgımız için argüman yollamamıza gerek yoktur. 
        char *args[] = {NULL};
		execvp("./reader", args);	

			
		}
	}
	return (0);
}
