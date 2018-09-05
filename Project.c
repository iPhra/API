#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int assets = 0; //variabile che conta il numero di risorse totali create
int counter; //variabile che conta il numero di risorse trovate in una find

struct son { //struct che rappresenta i figli di una data directory
	char* pathname; //pathname completo dalla radice
	struct directory* dir_address; //se la struct rappresenta una directory allora punta alla relativa struct directory
	struct file* file_address; //se la struct rappresenta un file allora punta al relativo struct file
	struct son* previous; //puntatore al precedente figlio nella lista
	struct son* next; //puntatore al successivo figlio nella lista
};

struct file { //struct che rappresenta i file
	char* pathname; //nome della risorsa
	int depth; //profondità del file nell'albero
	struct directory* father; //puntatore al padre
	char *contenuto; //contenuto del file
};

struct directory { //struct che rappresenta le risorse all'interno dell'albero
	char* pathname; //nome della risorsa
	int depth; //profondità della directory nell'albero
	int hashsize; //numero di figli (su 1024) che ha questa directory
	struct son* son[103]; //tabella hash grande 103, è un numero primo relativamente lontano da potenze di 2 (64 e 128) e che produce mediamente 10 collisioni
	struct directory* father; //puntatore al padre
};

int compare(const void* n1, const void* n2) { //funzione di comparison passata al quicksort
	char *string1 = *(char **)n1;
	char *string2 = *(char **)n2;
	return strcmp(string1, string2);
}

int getkey(char * string) { //data una stringa ritorna la key della hash corrispondente (codifica ASCII, metodo della divisione modulo 103)
	int result = 0;
	if (string == NULL)
		return 0;
	int length = strlen(string);
	for (int i = 0; i < length; i++) {
		result += string[i]; //sommo i vari caratteri che formano la stringa, sono implicitamente castati ad interi

		result = result % 103;
	}
	return result;
}

char* getpath(struct son* asset) { //data una risorsa restituisce il nome completo in formato unix
	struct directory* father = (asset->dir_address != NULL) ? asset->dir_address->father : asset->file_address->father;
	char* path = malloc(strlen(asset->pathname) + 1);
	char* tmp;
	strcpy(path, asset->pathname);
	while (father != NULL) {
		tmp = path;
		path = malloc((strlen(path) + strlen(father->pathname) + 2) * sizeof(char)); //alloco lo spazio necessario a congiungere il name del padre, del figlio, del terminatore e di /
		sprintf(path, "%s/%s", father->pathname, tmp); //unisco le due stringhe
		free(tmp);
		father = father->father; //salgo nell'albero
	}
	return path;
}

void file_insert(char *oldname, struct directory* tree) { //inserisce il file all'interno della struttura fornita
	char *name = malloc((strlen(oldname) + 1) * sizeof(char));
	strcpy(name, oldname); //copia di stringa in input
	int key = getkey(name);
	struct son* newfile = malloc(sizeof(struct son));
	struct son* previous = tree->son[key];
	struct file* file = malloc(sizeof(struct file));
	file->father = tree; //assegno i campi del nuovo file creato
	file->pathname = name;
	file->depth = tree->depth + 1;
	file->contenuto = "";
	tree->hashsize += 1; //aumento il numero di figli del padre
	assets += 1; //aumento le risorse dell'albero
	if (previous == NULL) { //se nella hash ho una lista vuota nella posizione key
		tree->son[key] = newfile;
		newfile->next = NULL;
		newfile->previous = NULL;
		newfile->dir_address = NULL;
		newfile->file_address = file;
		newfile->pathname = name;
	}
	else { //se nella hash non ho una lista vuota ma devo inserirlo in testa alla lista
		previous->previous = newfile;
		newfile->next = previous;
		tree->son[key] = newfile;
		newfile->previous = NULL;
		newfile->dir_address = NULL;
		newfile->file_address = file;
		newfile->pathname = name;
	}
	return;
}

void dir_insert(char *oldname, struct directory* tree) { //inserisce la directory all'interno della struttura fornita, stessi commenti di file_insert
	char *name = malloc((strlen(oldname) + 1) * sizeof(char));
	strcpy(name, oldname);
	int key = getkey(name);
	struct son* newdir = malloc(sizeof(struct son));
	struct son* previous = tree->son[key];
	struct directory* directory = malloc(sizeof(struct directory));
	tree->hashsize += 1; //aumento il numero di figli del padre
	assets += 1; //aumento il numero di risorse
	directory->father = tree; //riempio i campi della nuova directory
	directory->hashsize = 0;
	directory->depth = tree->depth + 1;
	directory->pathname = name;
	for (int i = 0; i < 103; i++) {
		directory->son[i] = NULL;
	}
	if (previous == NULL) { //se nella hash ho una lista vuota nella posizione key
		tree->son[key] = newdir;
		newdir->next = NULL;
		newdir->previous = NULL;
		newdir->dir_address = directory;
		newdir->file_address = NULL;
		newdir->pathname = name;
	}
	else { //se nella hash non ho una lista vuota ma devo inserirlo in testa alla lista
		previous->previous = newdir;
		newdir->next = previous;
		tree->son[key] = newdir;
		newdir->previous = NULL;
		newdir->dir_address = directory;
		newdir->file_address = NULL;
		newdir->pathname = name;
	}
	return;
}

struct file* find_file(char* name, struct directory* tree) { //trova il file name all'interno dei figli della directory tree
	int key = getkey(name);
	struct son* current = tree->son[key];
	while (current != NULL) {
		if ((strcmp(name, current->pathname) == 0) && (current->file_address != NULL)) {
			return current->file_address;
		}
		current = current->next;
	}
	return NULL;
}

struct directory* find_dir(char* name, struct directory* tree) { //cerca la directory name all'interno dei figli della directory fornita
	int key = getkey(name);
	struct son* current = tree->son[key];
	while (current != NULL) {
		if ((strcmp(name, current->pathname) == 0) && (current->dir_address != NULL)) {
			return current->dir_address;
		}
		current = current->next;
	}
	return NULL;
}

void create(char *input, struct directory* tree) {
	struct directory* current_dir = tree;
	struct directory* next_dir;
	char* current_name = strtok(input, "/");
	char* next_name;
	while (1) {
		next_dir = find_dir(current_name, current_dir); //se esiste, trova la directory corrispondente alla risorsa attuale
		next_name = strtok(NULL, "/"); //trova la risorsa successiva nel path fornito
		if (next_dir == NULL) {
			if (next_name == NULL && find_file(current_name, current_dir) == NULL) { //se ho finito di leggere l'input (sono al nome del file) e l'ultima risorsa (next_dir) non è una directory, né un file esistente, allora creo il file
				if ((current_dir->hashsize > 1023) || (current_dir->depth>254) || (strlen(current_name)>255)) { //condizioni sull'albero
					printf("no\n");
					return;
				}
				file_insert(current_name, current_dir);
				printf("ok\n");
				return;
			}
			printf("no\n");
			return;
		}
		current_dir = next_dir;
		current_name = next_name;
	}
}

void create_dir(char *input, struct directory* tree) {
	struct directory* current_dir = tree;
	struct directory* next_dir;
	char* current_name = strtok(input, "/");
	char* next_name;
	while (1) {
		next_dir = find_dir(current_name, current_dir);
		next_name = strtok(NULL, "/");
		if (next_dir != NULL && next_name == NULL) { //se l'ultima risorsa (il nome della directory) è già presente allora non la ricreo
			printf("no\n");
			return;
		}
		if (next_dir == NULL) {
			if (next_name == NULL) { //se l'ultima risorsa non è presente allora creo la directory
				if ((current_dir->hashsize > 1023) || (current_dir->depth>254) || (strlen(current_name)>255)) { //controlli sull'albero
					printf("no\n");
					return;
				}

				dir_insert(current_name, current_dir);
				printf("ok\n");
				return;
			}
			printf("no\n");
			return;
		}
		current_dir = next_dir;
		current_name = next_name;
	}
}

void read(char *input, struct directory* tree) {
	struct directory* current_dir = tree;
	struct directory* next_dir;
	struct file* file;
	char* current_name = strtok(input, "/");
	char* next_name;
	while (1) {
		next_name = strtok(NULL, "/");
		next_dir = find_dir(current_name, current_dir);
		if (next_dir == NULL) {
			if (next_name == NULL && strlen(current_name) <= 255) { //se l'ultima risorsa è il nome di un file ed esso è presente allora ci leggo/scrivo sopra
				file = find_file(current_name, current_dir);
				if (file != NULL) {
					printf("contenuto %s\n", file->contenuto);
					return;
				}
			}
			printf("no\n");
			return;
		}
		if (next_dir != NULL && next_name == NULL) { //se l'ultima risorsa è una directory non posso leggere/scriverci sopra
			printf("no\n");
			return;
		}
		current_dir = next_dir;
		current_name = next_name;
	}
}

void write(char *input1, char *input2, struct directory* tree) { //leggasi commenti alla read
	char *contenuto = malloc((strlen(input2) + 1) * sizeof(char));
	strcpy(contenuto, input2);
	struct directory* current_dir = tree;
	struct directory* next_dir;
	struct file* file = malloc(sizeof(struct file));
	char* current_name = strtok(input1, "/");
	char* next_name;
	while (1) {
		next_name = strtok(NULL, "/");
		next_dir = find_dir(current_name, current_dir);
		if (next_dir == NULL) {
			if ((next_name == NULL) && (strlen(current_name) <= 255)) {
				file = find_file(current_name, current_dir);
				if (file != NULL) {
					file->contenuto = contenuto;
					printf("ok %d\n", strlen(contenuto));
					return;
				}
			}
			printf("no\n");
			return;
		}
		if (next_dir != NULL && next_name == NULL) {
			printf("no\n");
			return;
		}
		current_dir = next_dir;
		current_name = next_name;
	}
}

void delete(char *input, struct directory* tree) {
	struct directory* current_dir = tree;
	struct directory* next_dir;
	struct file* file;
	struct son* current_son;
	char* current_name = strtok(input, "/");
	char* next_name;
	int key;
	while (1) {
		next_dir = find_dir(current_name, current_dir);
		next_name = strtok(NULL, "/");
		if (next_dir != NULL && next_name == NULL) { //devo cancellare una directory
			if (next_dir->hashsize == 0) { //posso farlo solo se non ha figli
				key = getkey(current_name);
				current_son = current_dir->son[key];
				while (1) { //sistemazione delle liste
					if (current_son != NULL) {
						if (current_son->dir_address == next_dir) {
							current_dir->hashsize -= 1;
							if (current_son->previous != NULL) {
								current_son->previous->next = current_son->next;
							}
							else current_dir->son[key] = current_son->next;
							if (current_son->next != NULL) {
								current_son->next->previous = current_son->previous;
							}
							printf("ok\n");
							free(next_dir->pathname);
							free(next_dir);
							free(current_son);
							return;
						}
						current_son = current_son->next;
					}
				}
			}
			printf("no\n");
			return;
		}
		if (next_dir == NULL) { //se non trovo una prossima directory o non esiste il percorso o devo cancellare un file
			file = find_file(current_name, current_dir);
			if (next_name == NULL && file != NULL) { //devo cancellare un file
				key = getkey(current_name);
				current_son = current_dir->son[key];
				while (1) {
					if (current_son != NULL) {
						if (current_son->file_address == file) {
							current_dir->hashsize -= 1;
							if (current_son->previous != NULL) {
								current_son->previous->next = current_son->next;
							}
							else current_dir->son[key] = current_son->next;
							if (current_son->next != NULL) {
								current_son->next->previous = current_son->previous;
							}
							printf("ok\n");
							free(file->pathname);
							if (strcmp(file->contenuto, "") != 0) free(file->contenuto);
							free(current_son);
							free(file);
							return;
						}
						current_son = current_son->next;
					}
				}
			}
			printf("no\n");
			return;
		}
		current_dir = next_dir;
		current_name = next_name;
	}
}

int delete_r(char *input, struct directory* tree) {
	struct directory* current_dir = tree;
	struct directory* next_dir;
	struct file* file;
	struct son* current_son;
	char* current_name = strtok(input, "/");
	char* next_name;
	int key;
	while (1) {
		next_dir = find_dir(current_name, current_dir);
		next_name = strtok(NULL, "/");
		if ((next_dir != NULL) && (next_name == NULL)) {  //devo cancellare una directory
			if (next_dir->hashsize == 0) { //se non ha figli faccio come per la delete normale
				key = getkey(current_name);
				current_son = current_dir->son[key];
				while (1) { //gestione delle liste
					if (current_son != NULL) {
						if (current_son->dir_address == next_dir) {
							current_dir->hashsize -= 1;
							if (current_son->previous != NULL) {
								current_son->previous->next = current_son->next;
							}
							else current_dir->son[key] = current_son->next;
							if (current_son->next != NULL) {
								current_son->next->previous = current_son->previous;
							}
							current_son->next = NULL;
							free(current_son);
							free(next_dir->pathname);
							free(next_dir);
							return 1;
						}
						current_son = current_son->next;
					}
				}
			}
			else { //se invece ha figli prima chiamo ricorsivamente la delete_r su di essi, poi elimino me stesso
				for (int i = 0; i < 103; i++) {
					struct son* garbage = next_dir->son[i]; //il figlio da eliminare

					while (garbage != NULL) {

						char* newpath = malloc((strlen(garbage->pathname) + 2) * sizeof(char));
						sprintf(newpath, "/%s", garbage->pathname);
						garbage = garbage->next;
						delete_r(newpath, next_dir);
						free(newpath);

					}
				} //ora elimino me stesso
				key = getkey(current_name);
				current_son = current_dir->son[key];

				for (; current_son != NULL; current_son = current_son->next) {  //gestione della lista di collisione
					if (current_son->dir_address == next_dir) {
						current_dir->hashsize -= 1;
						if (current_son->previous != NULL) {
							current_son->previous->next = current_son->next;
						}
						else current_dir->son[key] = current_son->next;
						if (current_son->next != NULL) {
							current_son->next->previous = current_son->previous;
						}
						current_son->next = NULL;
						free(current_son);
						free(next_dir->pathname);
						free(next_dir);
						return 1;
					}

				}

			}
		}
		if (next_dir == NULL) { //altrimenti devo eliminare un file oppure la directory non esiste
			file = find_file(current_name, current_dir);
			if (next_name == NULL && file != NULL) {
				key = getkey(current_name);
				current_son = current_dir->son[key];
				for (; current_son != NULL; current_son = current_son->next) {  //gestione della lista di collisione
					if (current_son->file_address == file) {
						current_dir->hashsize -= 1;
						if (current_son->previous != NULL) {
							current_son->previous->next = current_son->next;
						}
						else current_dir->son[key] = current_son->next;
						if (current_son->next != NULL) {
							current_son->next->previous = current_son->previous;
						}
						current_son->next = NULL;
						free(file->pathname);
						if (strcmp(file->contenuto, "") != 0) free(file->contenuto);
						free(file);
						free(current_son);
						return 1;
					}

				}

			}
			printf("no\n");
			return 0;
		}
		current_dir = next_dir;
		current_name = next_name;
	}
}

void find(char *input, struct directory* tree, char **result) {
	if (strlen(input) > 255) { //condizioni sul nome della risorsa
		return;
	}
	struct son* current_son;
	for (int i = 0; i < 103; i++) { //visito tutto l'albero
		current_son = tree->son[i];
		while (current_son != NULL) {
			if (current_son->dir_address != NULL) { //se sto puntanto ad una directory e non ad un file
				find(input, current_son->dir_address, result); //richiamo la find sulla directory 
			}
			if (strcmp(current_son->pathname, input) == 0) { //se la directory o il file hanno il nome cercato allora li metto nell'array e incremento il contatore
				result[counter] = getpath(current_son);
				counter++;
			}
			current_son = current_son->next;
		}
	}

	return;
}


int main() {
	char *input, *tok1, *tok2, *tok3;
	struct directory root;
	root.pathname = ""; //inizializzo la radice dell'albero
	root.father = NULL;
	root.depth = 0;
	root.hashsize = 0;
	for (int i = 0; i < 103; i++) {
		root.son[i] = NULL;
	}

	input = (char *)malloc(10000 * sizeof(char));
	while (1) {
		input = fgets(input, 10000, stdin); //leggo una linea di codice fino al carattere di new line, il valore 1000 è arbitrario, non sapendo quanto può essere lungo il contenuto di write non ho voluto scomodare realloc
		input[strlen(input) - 1] = '\0';
		if (strcmp(input, "exit") == 0) break; //se leggo exit esco

		tok1 = strtok(input, " "); //comando
		tok2 = strtok(NULL, " "); //percorso
		tok3 = strtok(NULL, "\""); //contenuto della write
		if (strcmp(tok1, "create") == 0) { //seguono un blocco di if-else per capire che comando ho letto e che funzione chiamare
			create(tok2, &root);
		}
		else if (strcmp(tok1, "create_dir") == 0) {
			create_dir(tok2, &root);
		}
		else if (strcmp(tok1, "read") == 0) {
			read(tok2, &root);
		}
		else if (strcmp(tok1, "write") == 0) {
			write(tok2, tok3, &root);
		}
		else if (strcmp(tok1, "delete") == 0) {
			delete(tok2, &root);
		}
		else if (strcmp(tok1, "delete_r") == 0) {
			int value = delete_r(tok2, &root);
			if (value == 1) printf("ok\n");
		}
		else if (strcmp(tok1, "find") == 0) {
			counter = 0;
			char **result = malloc(assets * sizeof(char*)); //alloco un array pari al numero di risorse della struttura, conterrà le stringhe trovate dalla find per poi ordinarle
			for (int i = 0; i < assets; i++) {
				result[i] = NULL;
			}
			find(tok2, &root, result);
			if (counter == 0) { //non ho trovato nessuna risorsa col nome cercato
				printf("no\n");
				continue;
			}
			qsort(result, counter, sizeof(char *), compare); //ordino l'array col quicksort
			for (int i = 0; i < counter && result[i] != NULL; i++) { //stampo gli elementi dell'array rilevanti
				printf("ok %s\n", result[i]);
				free(result[i]);
			}
		}
	}
	return 0;
}

