#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <iostream>
#include <regex.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <locale.h>

using namespace std;

int createdSocket, createdSocket2, c;
string adresaServeru;
string userName;
string userPassword;
int port = 21;
string path;
string celaAdresa;
int pocetPresmerovani = 0;

int zpracujAdresu(int argc, char *argv[])
{
	celaAdresa.clear();
	celaAdresa.append(argv[argc-1]);
	userName.clear();
	userPassword.clear();
	adresaServeru.clear();
	int nam = 1;
	int pas = 1;
	int i = 0;
	int zac = 0;

	if (celaAdresa.compare(0,6, "ftp://") == 0) //pokial adresa zacina na ftp://
	{
		i = 6;
		zac = 1;
		if (celaAdresa.compare(6,4, "ftp.") == 0) //pokiald je ftp
		{
			nam = 0;		 //ak je priamo zadana hodnota ftp a nieje zadane meno a heslo hosta je potrebne pouzit 							anonymne prihlasenie
			pas = 0;
			i = 10;
			adresaServeru.append("ftp.");
			}
	}
	else if (celaAdresa.compare(0,4, "ftp.") == 0)       //ak je adresa bez ftp:// a hned nasleduje ftp.
	{
		i = 4;
		adresaServeru.append("ftp.");
		nam = 0;		//ak je priamo zadana hodnota ftp a nieje zadane meno a heslo hosta je potrebne pouzit anonymne prihlasenie
		pas = 0;
	}

	if (nam == 0 && pas == 0)
	{
		userName.append("anonymous");
		userPassword.append("secret");
	}
	int p = i;
	while(i < celaAdresa.length()) //nacitaj hosta
	{
		string tmpStr;
		tmpStr.clear();
		tmpStr += celaAdresa.at(i);
		if (celaAdresa.compare(i,1, "@") == 0)
		{
			if(zac == 0)
			{
				fprintf(stderr, " Musis pridat ftp:// na zaciatok !\n");
				return -1;
			}
			if(nam != 0 && pas!=0)
			{
			int wrname = 1;
			int wrpass = 0;
				while(p<i) // rozdelenie a ulezenie username a pass
				{
					if (celaAdresa.compare(p,1, ":") == 0)
					{
						wrname = 0;
						wrpass = 1;
					}
					else if(wrname == 1)
					{
						userName += celaAdresa.at(p);					
					}
					else if(wrpass == 1)
					{
						userPassword +=	celaAdresa.at(p);
					}
					p++;
				}
				if(userName.length() == 0 ){
					fprintf(stderr, " Chyba meno uzivatele !\n");
					return -1;
				}
				else if(userPassword.length() == 0 ){
					fprintf(stderr, "Chyba heslo uzivatele !\n");
					return -1;
				}
			break;
			}
		}

		if(nam == 0 && pas == 0)
		{
			if (celaAdresa.compare(i,1, ":") == 0) //pokial bolo zadane cislo portu
				break;
		}		
		
		if (celaAdresa.compare(i,1, "/") == 0)
			break;

		if (isspace(celaAdresa.at(i)))
		{
			fprintf(stderr, "Medzera v adrese!\n");
			return -1;
		}
		adresaServeru += tmpStr;
		i++;
	}
//////////////////Nacitanie adresy
	if(celaAdresa.compare(i,1, "@") == 0)
	{
		i++;
		if(celaAdresa.compare(i,4, "ftp.") == 0)
		{
			adresaServeru.clear();
			while(i < celaAdresa.length())
			{
				string tmpStr;
				tmpStr.clear();
				if (celaAdresa.compare(i,1, "/") == 0) //zadany path
					break;
				if (celaAdresa.compare(i,1, ":") == 0) //pokial bolo zadane cislo portu
					break;
				if (isspace(celaAdresa.at(i)))
				{
					fprintf(stderr, "Medzera v adrese!\n");
					return -1;
				}
				tmpStr += celaAdresa.at(i);
				adresaServeru += tmpStr;
				i++;
			}
		}
		else
		{
			fprintf(stderr, " Chyba meno ftp serveru !\n");
			return -1;
		}
	}
////////////////////////////////////////////PORTTTTTTPORTTTT
	if (celaAdresa.compare(i,1, ":") == 0) //nacitam cislo portu
	{
		i++;
		if(celaAdresa.compare(i,2, "21") == 0)
			port = 21;
		else 
		{
			fprintf(stderr, "Zly port pouzite port 21\n");
			return -1;
		}
		i+=2;
		if(i < celaAdresa.length() )
		{
			if (celaAdresa.compare(i,1, "/") != 0)
			{
				fprintf(stderr, "Zle zadane parametre!\n");
				return -1;
			}
		}
	}
//////////////////////////////////////////////////////////////PATH
	if (celaAdresa.compare(i,1, "/") == 0)
	{
		path.clear();
			while (i < celaAdresa.length()) //nacita zostatok adresy PATH
			{
				string tmpStr;
				tmpStr.clear();
				tmpStr += celaAdresa.at(i);
				path += tmpStr;
				i++;
			}
	}
return 0;
}

////////////////////////////maiiiiiinnnn
int main(int argc, char *argv[])
{
	if(zpracujAdresu(argc, argv) != 0) //spracujem adresu
		return EXIT_FAILURE;
	
	struct sockaddr_in in_socket, in_socket2;
	in_socket.sin_port = htons(port);
	in_socket.sin_family = AF_INET;

	struct hostent *strc;

	if((createdSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) //spravim si socket
	{
		fprintf(stderr, "Nepodarilo sa vytvorit soket.");
		return EXIT_FAILURE;
	}

	if((strc = gethostbyname(adresaServeru.c_str())) == NULL) //prelozim adresu
	{
		fprintf(stderr, "Nepodaril sa vytvorit hosta.");
		return EXIT_FAILURE;
	}

	memcpy(&in_socket.sin_addr, strc->h_addr_list[0], strc->h_length);

	if(connect(createdSocket, (struct sockaddr*) &in_socket, sizeof(in_socket)) < 0) //spojim sa
	{
		fprintf(stderr, "Nepodarilo sa spojit.");
		return EXIT_FAILURE;
	}    

	char server_reply[20000]={0}, server_reply2[2000]={0}, server_reply3[2000]={0}, server_reply4[2000]={0}, server_reply6[2000]={0};
	recv(createdSocket, server_reply , 2000 , 0);
//Poziadavok na pripojenie Hosta pouzitim hostname
	string meno;
	meno.append("USER ");
	meno.append(userName);
	meno.append("\r\n");
	int delkaZpravy = meno.length();

	if( send(createdSocket , meno.c_str() , delkaZpravy , 0) < 0)
	{
		fprintf(stderr, "Nepodarilo sa poslat poziadavok UserName!\n");
		return EXIT_FAILURE;
	}

	if( recv(createdSocket, &server_reply , 2000 , 0) < 0)
	{
		fprintf(stderr, "Chyba odpovede na poziadavok UserName!\n");
		return EXIT_FAILURE;
	}
//Poziadavok na pripojenie Hesla k danemu hostovi pouzitim password
	string heslo;
	heslo.append("PASS ");
	heslo.append(userPassword);
	heslo.append("\r\n");
	delkaZpravy = heslo.length();
   
	if( send(createdSocket , heslo.c_str() , delkaZpravy , 0) < 0)
	{
		fprintf(stderr, "Nepodarilo sa poslat poziadavok UserPassword!\n");
		return EXIT_FAILURE;
	}
    
	if( recv(createdSocket, &server_reply ,  2000 , 0) < 0)
	{
		fprintf(stderr, "Chyba odpovede na poziadavok UserName!\n");
		return EXIT_FAILURE;
	}

	sleep(2);
	if(path.length() > 0)
	{
		string adresar;
		adresar.append("CWD ");
		adresar.append(path);
		adresar.append("\r\n");
		delkaZpravy = adresar.length();

		if( send(createdSocket , adresar.c_str() , delkaZpravy , 0) < 0)
		{
			fprintf(stderr, "Nepodarilo sa poslat poziadavok CD!\n");
			return EXIT_FAILURE;
		}
    
		if( recv(createdSocket, &server_reply ,  2000 , 0) < 0)
		{
			fprintf(stderr, "Chyba odpovede na poziadavok UserName!\n");
			return EXIT_FAILURE;
		}
	}

	string daco;
	daco.append("\r\n");
	delkaZpravy = daco.length();

	if( send(createdSocket , daco.c_str() , delkaZpravy , 0) < 0)
	{
		fprintf(stderr, "Nepodarilo sa poslat poziadavok!\n");
		return EXIT_FAILURE;
	}
    
	if( recv(createdSocket, server_reply ,  2000 , 0) < 0)
	{
		fprintf(stderr, "Chyba odpovede na poziadavok!\n");
		return EXIT_FAILURE;
	}

	sleep(2);

	string epsv;
	epsv.append("EPSV");
	epsv.append("\r\n");
	delkaZpravy = epsv.length();

	if( send(createdSocket , epsv.c_str() , delkaZpravy , 0) < 0)
	{
		fprintf(stderr, "Nepodarilo sa poslat poziadavok EPSV!\n");
		return EXIT_FAILURE;
	}
    
	if( recv(createdSocket, server_reply4 ,  1024 , 0) < 0)
	{
		fprintf(stderr, "Chyba odpovede na poziadavok!\n");
		return EXIT_FAILURE;
	}
	sleep(1);

////////////////////////////////////////////NACITANIe PORTU 2 Koli primaniu
	char port_second[2000];
	int i=0,j=0;
	int length_port= strlen(server_reply4);

	for(i=4; i <= length_port; i++)
	{
		if(isdigit(server_reply4[i]))
		{
			port_second[j]=server_reply4[i];
			j++;
		}
	}
	int port_second_socket= atoi(port_second);

//////////////////////////////////////Vytvorim 2 soket
	in_socket2.sin_port = htons(port_second_socket); 
	in_socket2.sin_family = AF_INET; 

	if((createdSocket2 = socket(AF_INET, SOCK_STREAM, 0)) < 0) //vytvorym 2 soket
	{ 
		fprintf(stderr, "Nepodarilo se vytvorit 2 soket!\n"); 
		return EXIT_FAILURE; 
	} 
	
	memcpy(&in_socket2.sin_addr, strc->h_addr_list[0], strc->h_length); 

	if(connect(createdSocket2, (struct sockaddr*) &in_socket2, sizeof(in_socket2)) < 0)  //pripojim sa na 2 soket
	{ 
		fprintf(stderr, "Nepodarilo sa spojit soket2. \n"); 
		return EXIT_FAILURE;
	}

	string list;
	list.append("LIST");
	list.append("\r\n");
	delkaZpravy = list.length();

	if( send(createdSocket , list.c_str() , delkaZpravy , 0) < 0)
	{
		fprintf(stderr, "Nepodarilo sa poslat poziadavok LIST!\n");
		return EXIT_FAILURE;
	}

	sleep(1);

	string prijataHlavicka;
	bool indic = false; 
	char sav; 
	int inc=0; 
	for(;;) 
	{ 
		if(read(createdSocket2, &sav, 1) < 0) 
		{ 
			fprintf(stderr, "Chyba pri citani."); 
			return EXIT_FAILURE;
		} 
		inc++; 
		prijataHlavicka.append(&sav, 1); 
		if(isspace(sav) == 0) 
			indic = true; 
		if('\n' == sav) 
		{ 
			if(indic) 
			{
				indic = false;
				continue; 
			} 
			break; 
		}
	}

	sleep(1);

	if(close(createdSocket) < 0)  //Pokusim sa uzavriet soket
	{
		fprintf(stderr, "Nepodarilo sa zavriet soket");
		return EXIT_FAILURE;
	}
	prijataHlavicka = prijataHlavicka.erase(prijataHlavicka.find_last_not_of(' '));
	std::cout << prijataHlavicka.c_str();
	return 0;
}










