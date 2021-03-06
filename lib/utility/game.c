#include <stdio.h>
#include <stdlib.h>
#include "../utility/utility.h"

Word Config = {"config.txt", 10};

void startGame(Game *game){
  printf("Selamat datang di Mobilita!\nSilahkan masukkan nama config file: ");
  printf("\n");
  startWord();
  Word cfg = currentWord;
  currentConfigFile = cfg;
  inputConfigFile(game, cfg);
  game->endGame = false;
  currentTime = 0;
  currentLocation = game->hq;
  currentMoney = 0;
  totalMoney = 0;
  finishedPesanan = 0;
  CreateLinkedList(&TODO); // buat to do list
  CreateLinkedList(&inProgress); // buat  inprogress
  CreateStack(&game->tas); // buat tas baru
  createInventory(&game->gl); // ini buat new inven
  gadgetInfo(&Toko); // ini inisialisasi toko
  createAbility(&game->b);
}

// void startGame(Game *game){
//   printf("Selamat datang di Mobilita!\nSilahkan masukkan nama config file: ");
//   printf("\n");
//   // startWord();
//   // Config = currentWord;
//   currentConfigFile = Config;
//   inputConfigFile(game, Config);
//   game->endGame = false;
//   currentTime = 51;
//   currentLocation = game->hq;
//   currentMoney = 60000;
//   CreateLinkedList(&TODO); // buat to do list
//   CreateLinkedList(&inProgress); // buat  inprogress
//   CreateStack(&game->tas); // buat tas baru
//   createInventory(&game->gl); // ini buat new inven
//   gadgetInfo(&Toko); // ini inisialisasi toko
//   createAbility(&game->b);
// }


void startGameFromLoad(Game *game){
  printf("Selamat datang di Mobilita!\nSilahkan masukkan nama load file: ");
  printf("\n");
  startWord();
  Word cfg = currentWord;
  game->endGame = false;
  CreateLinkedList(&TODO); // buat to do list
  CreateLinkedList(&inProgress); // buat  inprogress
  CreatePrioQueue(&psnTerurut);
  CreateStack(&game->tas); // buat tas baru
  createInventory(&game->gl); // ini buat new inven
  gadgetInfo(&Toko); // ini inisialisasi toko
  createAbility(&game->b);
  loadGame(game, cfg);
}


void buy(Game *g){
  if(EQ(currentLocation.koor, g->hq.koor)){
    buyGadget(Toko, &g->gl, &currentMoney);
  }else{
    printf("Tidak bisa membeli item karena tidak di HQ.\n");
  }
}

void progress(Game *g){
  if(!isStackEmpty(g->tas)){
    Stack tasTemp;
    Pesanan temp;
    Pesanan temp2;
    int len = IDX_TOP(g->tas);
    CreateStack(&tasTemp);
    for(int i = 0; i <= len; i++){
      pop(&g->tas, &temp);
      if(TYPE(tipeItem(temp)) == 'P'){
        minusExpiry(&temp.tipeItem);
        minusExpiryList(&inProgress);
        if(temp.tipeItem.expiry_now <= 0){
          if(checkInProgress(inProgress, temp) != -1){
            deletePesanan(&inProgress, &temp2, checkInProgress(inProgress, temp));
          }
        }else{
          push(&tasTemp, temp);
        }
      }else{
        push(&tasTemp, temp);
      }
    }
    len = IDX_TOP(tasTemp);
    //NOTE Ngebalikin lagi soalnya tadi pasti kebalik hasilnya
    for(int i = 0; i <= len; i++){
      pop(&tasTemp, &temp);
      push(&g->tas, temp);
    }
  }
}

boolean checkEndGame(Game g){
    return isStackEmpty(g.tas) && isLinkedListEmpty(TODO) && isLinkedListEmpty(inProgress) && isEmpty(psnTerurut) && isLocationEqual(currentLocation, g.hq);
}

void endGameReached(Game g){
    sprint_yellow("SELAMAT! Anda berhasil menamatkan game MOBILITA!\n");
    sprint_yellow("Dengan ini, kamu berhasil pensiun dari pekerjaanmu sebagai kurir!\n");
    sprint_yellow("Pandemi telah usai, mobita pun kembali masuk kampus..\n\n");
    sprint_yellow("Finished stats: \n");
    printf("Pesanan yang telah diantar: %i", finishedPesanan); printf("\n");
    printf("Waktu yang terlampaui: %i", currentTime); printf("\n");
    printf("Total uang yang didapat: %i", totalMoney); printf("\n");
    sprint_yellow("Sampai jumpa di lain waktu!\n");
}

void quitWithSave(Game g){
    Word Y = {"Y", 1};
    sprint_red("Apakah anda ingin menyimpan permainan sebelum quit? (Y/N): ");
    startWord();
    if(isWordEqual(Y, currentWord)){
        saveGame(g, currentConfigFile);
    }
    sprint_red("Sampai jumpa di lain waktu!\n");
    exit(0);
}

void move(Game *g){
    Lokasi possibleMoves[26] = {0};
    int i;
    int idx;
    int count = 0;
    // Checking index
    for(i = 0; i < 27; i++){
        if(EQ(g->bangunan.buffer[i].koor, currentLocation.koor)){
            idx = i;
        }
    }

    // Searching possible moves
    for(i = 0; i < g->adj.colEff; i++){
        if(g->adj.contents[idx][i] == 1){
            possibleMoves[count] = g->bangunan.buffer[i];
            count++;
        }
    }

    // Printing possible moves
    printf("Posisi yang dapat dicapai: \n");
    for(i = 0; i < count; i++){
        printf("%d. %c ", i+1, possibleMoves[i].tipeBangunan);  TulisPOINT(possibleMoves[i].koor); printf("\n");
    }

    // Execute moves
    printf("Posisi yang dipilih? (ketik 0 jika ingin kembali)\n");
    printf("ENTER COMMAND: ");
    startWord();
    int choice = atoi(currentWord.contents);
    if(choice > count || choice < 0){
        printf("Input tidak valid. Mengembalikan ke menu utama.\n");
        return;
    }
    else{
        // Update variables
        if(choice == 0){
            return;
        }
        else{
            checkHeavyIteminBag(&g->b, g->tas);
            if(HEAVYITEM(g->b) && FREEZE(g->b)){
              TIME(g->b) = 0;
              deactivateSpeedBoost(&g->b);
            }
            if(FREEZE(g->b)){
              TIME(g->b)--;
              if(TIME(g->b) % 2 == 0 && TIME(g->b) < 10){
                currentTime++;
              }
              deactivateSpeedBoost(&g->b);
            }
            else if(HEAVY(g->tas) >= 1){
              currentTime = currentTime + HEAVY(g->tas) + 1;
            }
            else{
              currentTime++;
            }
            progress(g);
            updatePosition(possibleMoves[choice-1]);           
        }
    }
}

void pickup(Game *g){
  Pesanan inputPesanan;
  if(checkPesanan(TODO, currentLocation) == -1){
    printf("Tidak ada item pada bangunan ini.\n");
  }else{
    if(TOP(g->tas).tipeItem.type == 'V'){
      printf("Antarkan dulu VIP Itemnya!\n");
    }else if(!isStackFull(g->tas)){
      deletePesanan(&TODO, &inputPesanan, checkPesanan(TODO, currentLocation));
      push(&g->tas, inputPesanan);
      insertLast(&inProgress, inputPesanan);
      printf("Pesanan %s berhasil diambil.\n", TYPE_DESC(tipeItem(inputPesanan)));
    }else{
      printf("Tas sudah penuh tidak bisa mengambil barang lagi.\n");
    }
  }
}

int checkPossibleMoves(Game g, int *possibleMoves){
    int i;
    int idx;
    int count = 0;
    for(i = 0; i < 27; i++){
        if(EQ(g.bangunan.buffer[i].koor, currentLocation.koor)){
            idx = i;
        }
    }
    for(i = 0; i < g.adj.colEff; i++){
        if(g.adj.contents[idx][i] == 1){
            possibleMoves[count] = i;
            count++;
        }
    }
    return count;
}

void displayPeta(Game g, int time){
    int row = g.size.X;
    int col = g.size.Y;

    Matrix m;
    CreateMatrix(row, col, &m);
    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){
        MATRIXELMT(m, i, j) = -1;
        }
    }
    // printf("%i\n", g.bangunan.nEff);
    for(int i = 0; i < g.bangunan.nEff; i++){
        MATRIXELMT(m, g.bangunan.buffer[i].koor.X-1, g.bangunan.buffer[i].koor.Y-1) = i;
        // MATRIXELMT(m, ELMTADJMTRX(p, i).X-1, ELMTADJMTRX(p, i).Y-1) = i;
    }
    for(int i = 0; i < row-1; i++){
        if(i == row - 1){
        sprint_magenta("*");
        }else sprint_magenta("* ");
    }
    printf("\n");

    int possMoves[30];
    int possMoveCnt = checkPossibleMoves(g, possMoves);
    boolean green = false;
    boolean red = false;
    boolean blue = false;
    
    int loc;
    for(int i = 0; i < row; i++){
        for(int j = 0; j < col; j++){
            green = false;
            red = false;
            blue = false;
            loc = MATRIXELMT(m, i, j);
            if(j == 0){
                sprint_magenta("*");
            }
            if(loc != -1){
                for(int k = 0; k < possMoveCnt; k++){
                    if(loc == possMoves[k]){
                        green = true;
                    }
                }
                if(hasItem(TODO, g.bangunan.buffer[loc])){
                  red = true;
                }
                if(g.bangunan.buffer[loc].tipeBangunan == TOP(g.tas).dropOff){
                  blue = true;
                }
                
                if(isLocationEqual(currentLocation, g.bangunan.buffer[loc])){
                    print_yellow(g.bangunan.buffer[loc].tipeBangunan);
                }
                else if(blue){
                  print_blue(g.bangunan.buffer[loc].tipeBangunan);
                }
                else if(red){
                  print_red(g.bangunan.buffer[loc].tipeBangunan);
                }

                else if(green){
                    print_green(g.bangunan.buffer[loc].tipeBangunan);
                }
                
                else{
                    printf("%c", g.bangunan.buffer[loc].tipeBangunan);
                }
            }else printf(" ");
            if(j == col - 1){
                sprint_magenta("*");
            }
            }
            printf("\n");
    }
        // for(int i = 0; i < row; i++){
        //     for(int j = 0; j < col; j++){
                
        //     }
        // }

    for(int i = 0; i < row-1; i++){
        if(i == row - 1){
        sprint_magenta("*");
        }else sprint_magenta("* ");
    }
    printf("\n");
}

void displayCurrentLocation(){
    printf("Mobita sekarang berada di titik "); TulisLokasi(currentLocation);
}

void displayCurrentMoney(){
    printf("Uang yang dimiliki: %d Yen\n", currentMoney);
}



void updatePosition(Lokasi l){
    currentLocation.koor.X = l.koor.X;
    currentLocation.koor.Y = l.koor.Y;
    currentLocation.tipeBangunan = l.tipeBangunan;
}

void dropOff(Game *g){
    if (!isStackEmpty(g->tas)){
        if (getTipeBangunan(currentLocation) == DROPOFF(TOP(g->tas))){
            Pesanan dropped;
            deleteLast(&inProgress, &dropped);
            if(SENTERPENGECIL(g->b)){
              HEAVY(g->tas)++;
              SENTERPENGECIL(g->b) = false;
            }
            pop(&g->tas,&dropped);
            finishedPesanan++;
            totalMoney += VALUE(tipeItem(dropped));
            currentMoney += VALUE(tipeItem(dropped));
            switch (TYPE(tipeItem(dropped))){
                case 'V':
                    RETURNTOSENDER(g->b) = true;
                    break;
                case 'H':
                    checkHeavyIteminBag(&g->b, g->tas);
                    activateSpeedBoost(&g->b);
                    break;
                case 'P':
                    activateIncreaseCapacity();
                    break;
                default:
                    break;
            }
            printf("Pesanan %s berhasil diantarkan.\n", TYPE_DESC(tipeItem(dropped)));
            printf("Uang yang didapatkan: %d Yen\n", VALUE(tipeItem(dropped)));
        }else{
          printf("Bukan Lokasi yang tepat!\n");
        }
    }else{
        printf("Tidak ada pesanan yang dapat diantarkan!\n");
    }
}

PrioQueue transformToPrioQueue(Game g){
  PrioQueue pq;
  CreatePrioQueue(&pq);
  for(int i = 0; i < g.jumlah_pesanan; i++){
    enqueue(&pq, g.psn[i]);
  }
  return pq;
}

void pintuKemanaSaja(Game *g){
    int i;
    int idx;
    int count = listDinLength(g->bangunan);
    // Checking index
    printf("Posisi yang dapat dicapai: \n");
    for(i = 0; i < count; i++){
        printf("%d. %c ", i+1, g->bangunan.buffer[i].tipeBangunan);  TulisPOINT(g->bangunan.buffer[i].koor); printf("\n");
    }
    printf("Masukkan lokasi yang ingin dituju: ");
    startWord();
    int choice = atoi(currentWord.contents);
    if(choice > count || choice < 1){
        printf("Input tidak valid. Gadget terpakai.\n");
        return;
    }
    else{
        progress(g);
        updatePosition(g->bangunan.buffer[choice-1]);
    }
    printf("Pintu Kemana Saja berhasil digunakan!\n");
}

void senterPengecil(Game *g){
  if(TOP(g->tas).tipeItem.type == 'H'){
    SENTERPENGECIL(g->b) = true;
    HEAVY(g->tas)--;
    printf("Senter pengecil berhasil digunakan\n");
  }else{
    printf("JIAKH SIA SIA BOS HAHA\n");
  }
}

void displayInventory(GadgetList *I, Game *g){
    int user_input;
    for(int i=0; i<INVENTORY_CAP; i++){
        printf("%d. ", i+1);
        if(ID_GADGET((*I).buffer[i]) == ID_UNDEF){
            printf("-\n");
        }else{
            int ids = ID_GADGET((*I).buffer[i]);
            printf("%s\n", NAME((*I).buffer[i]));
        }
    }
    printf("Gadget mana yang ingin digunakan? (ketik 0 jika ingin kembali)\n");
    printf("\n");

    printf("ENTER COMMAND: ");
    startWord();
    user_input = atoi(currentWord.contents);
    if (user_input > 0 && user_input <= 5){
        int gadget_id = ID_GADGET((*I).buffer[user_input-1]);
        if (gadget_id != ID_UNDEF){
            if (gadget_id == 0){
                kainPembungkusWaktu(&g->tas, &inProgress);
                ID_GADGET((*I).buffer[user_input-1]) = ID_UNDEF;
            }
            else if(gadget_id == 1){
              senterPembesar(&stack_capacity);
              ID_GADGET((*I).buffer[user_input-1]) = ID_UNDEF;
            }
            else if(gadget_id == 2){
              pintuKemanaSaja(g);
              ID_GADGET((*I).buffer[user_input-1]) = ID_UNDEF;
            }
            else if(gadget_id == 3){
                mesinWaktu(&currentTime, &g->tas, &inProgress);
                ID_GADGET((*I).buffer[user_input-1]) = ID_UNDEF;
            }
            else if(gadget_id == 4){
                senterPengecil(g);
                ID_GADGET((*I).buffer[user_input-1]) = ID_UNDEF;
            }
        }else{
            printf("Tidak ada gadget yang dapat digunakan!\n");
        }
    }else if(user_input == 0 ){
        printf("Kembali ke menu\n");
        /* back to menu*/
    }
    else{
        printf("input tidak valid\n");
    }
}



void copyFromFile(Word cfg, Word savefile){
    char ch;
    Word NulTerm = {"\0", 1};
    FILE *source, *target;
    Word dir = {"./config/", 9};
    Word savepath = concatWord(dir, savefile);
    Word path = concatWord(dir, cfg);
    savepath = concatWord(savepath, NulTerm);
    path = concatWord(path, NulTerm);
    // source = fopen(path.contents, "r");
    source = fopen("./config/config.txt", "r");
    if(source == NULL){
        printf("Error: File source not found. Exiting..\n");
        exit(0);
    }
    // target = fopen(savepath.contents, "w");
    target = fopen("./config/coba.txt", "w");
    if(target == NULL){
        printf("Error: File not found. Exiting..\n");
        exit(0);
    }
    fprintf(target, "a");
    while((ch = fgetc(source)) != EOF){
        printf("%c\n", ch);
        fputc(ch, target);
    }
    fclose(source);
    fclose(target);
}

void saveGame(Game g, Word cfg){
    FILE *save;
    FILE *source;
    char ch;
    printf("Masukkan nama file penyimpanan: ");
    startWord();
    Word savefile = currentWord;
    Word dir = {"./config/", 9};
    Word savepath = concatWord(dir, savefile);
    Word NulTerm = {"\0", 1};
    Word path = concatWord(dir, cfg);
    savepath = concatWord(savepath, NulTerm);
    path = concatWord(path, NulTerm);
    source = fopen(path.contents, "r");
    // source = fopen("./config/config.txt", "r");
    if(source == NULL){
        printf("Error: File source not found. Exiting..\n");
        exit(0);
    }
    save = fopen(savepath.contents, "w");
    // save = fopen("./config/coba.txt", "w");
    if(save == NULL){
        printf("Error: File not found. Exiting..\n");
        exit(0);
    }
    while((ch = fgetc(source)) != EOF){
        fputc(ch, save);
    }
    fclose(source);
    // fclose(target);

    // fclose(save);
    // Curr Config File
    fprintf(save, "%s\n", currentConfigFile.contents);
    // Curr Time Curr Money
    fprintf(save, "%d %d\n", currentTime, currentMoney);
    // Finished, Total
    fprintf(save, "%d %d\n", finishedPesanan, totalMoney);
    // Curr Loc
    fprintf(save, "%c %d %d\n", currentLocation.tipeBangunan, currentLocation.koor.X, currentLocation.koor.Y);



    // fprintf(save, "TEMPAT TODO\n");
    // TODO
    int todolength = listLinkedLength(TODO);
    fprintf(save, "%d\n", todolength);
    for(int i = 0; i < todolength; i++){
        Pesanan tempP = getElmt(TODO,i);
        if(tempP.tipeItem.type == 'P'){
            fprintf(save, "%d %c %c %c %d\n", tempP.t, tempP.pickUp, tempP.dropOff, tempP.tipeItem.type, tempP.tipeItem.expiry);
        }
        else{
            fprintf(save, "%d %c %c %c\n", tempP.t, tempP.pickUp, tempP.dropOff, tempP.tipeItem.type);
        }
    }


    // fprintf(save, "TEMPAT InPro\n");
    // InProgress
    int inprolength = listLinkedLength(inProgress);
    fprintf(save, "%d\n", inprolength);
    if(inprolength > 0){
        for(int i = 0; i < inprolength; i++){
            Pesanan tempP = getElmt(inProgress,i);
            if(tempP.tipeItem.type == 'P'){
                fprintf(save, "%d %c %c %c %d %d\n", tempP.t, tempP.pickUp, tempP.dropOff, tempP.tipeItem.type, tempP.tipeItem.expiry, tempP.tipeItem.expiry_now);
            }
            else{
                fprintf(save, "%d %c %c %c\n", tempP.t, tempP.pickUp, tempP.dropOff, tempP.tipeItem.type);
            }
        }
    }

    // PsnTerurut
    int psnlength = lengthPrioQueue(psnTerurut);
    fprintf(save, "%d\n", psnlength);
    if(psnlength > 0){
        for(int i = 0; i < psnlength; i++){
            Pesanan tempP = psnTerurut.buffer[i];
            if(tempP.tipeItem.type == 'P'){
                fprintf(save, "%d %c %c %c %d %d\n", tempP.t, tempP.pickUp, tempP.dropOff, tempP.tipeItem.type, tempP.tipeItem.expiry, tempP.tipeItem.expiry_now);
            }
            else{
                fprintf(save, "%d %c %c %c\n", tempP.t, tempP.pickUp, tempP.dropOff, tempP.tipeItem.type);
            }
        }
    }


    // fprintf(save, "TEMPAT Tas\n");
    // Tas
    // idxTop, heavyItem
    fprintf(save, "%d %d\n", g.tas.idxTop, g.tas.heavyItem);
    if(inprolength > 0){
        for(int i = 0; i < inprolength; i++){
            Pesanan tempP = g.tas.buffer[i];
            if(tempP.tipeItem.type == 'P'){
                fprintf(save, "%d %c %c %c %d %d\n", tempP.t, tempP.pickUp, tempP.dropOff, tempP.tipeItem.type, tempP.tipeItem.expiry, tempP.tipeItem.expiry_now);
            }
            else{
                fprintf(save, "%d %c %c %c\n", tempP.t, tempP.pickUp, tempP.dropOff, tempP.tipeItem.type);
            }
        }
    }



    // fprintf(save, "TEMPAT GL\n");
    // GadgetList
    for(int i = 0; i < 5; i++){
        Gadget tempG = g.gl.buffer[i];
        fprintf(save, "%d\n", tempG.id);
    }


    // fprintf(save, "TEMPAT Abil\n");
    // Ability
    fprintf(save, "%d\n", g.b.time);
    if(g.b.freezeTime){
        fprintf(save, "%d\n", 1);
    }
    else{
        fprintf(save, "%d\n", 0);
    }
    if(g.b.heavyItem){
        fprintf(save, "%d\n", 1);
    }
    else{
        fprintf(save, "%d\n", 0);
    }
    if(g.b.returnToSender){
        fprintf(save, "%d\n", 1);
    }
    else{
        fprintf(save, "%d\n", 0);
    }
    if(g.b.isSenterPengecilActivated){
        fprintf(save, "%d\n", 1);
    }
    else{
        fprintf(save, "%d\n", 0);
    }

    fclose(save);
    sprint_blue("Game successfully saved.\n");
}


void loadGame(Game *g, Word cfg){
    Word EMPTY = {"", 0};
    currentWord = EMPTY;
    Word dir = {"./config/", 9};
    Word path = EMPTY;
    path = concatWord(dir, cfg);
    startFromFile(path.contents);
    CreateListDin(&g->bangunan, 30);
    advWord();
    g->size.X = atoi(currentWord.contents);
    advWord();
    g->size.Y = atoi(currentWord.contents);
    advNewline();
    g->hq.koor.X = atoi(currentWord.contents);
    advWord();
    g->hq.koor.Y = atoi(currentWord.contents);
    g->hq.tipeBangunan = '8';
    insertLastListDin(&g->bangunan, g->hq);
    advNewline();
    g->jumlah_lokasi = atoi(currentWord.contents);
    int i;
    Lokasi temploc;
    for(i = 1; i < g->jumlah_lokasi+1; i++){
        advNewline();
        temploc.tipeBangunan = currentWord.contents[0];
        advWord();
        temploc.koor.X = atoi(currentWord.contents);
        advWord();
        temploc.koor.Y = atoi(currentWord.contents);
        insertLastListDin(&g->bangunan, temploc);
    }
    g->adj.colEff = g->jumlah_lokasi+1;
    g->adj.rowEff = g->jumlah_lokasi+1;
    int j;
    advNewline();
    // displayWord(currentWord);
    for(i = 0; i < g->adj.rowEff; i++){
        for(j = 0; j < g->adj.colEff; j++){
            g->adj.contents[i][j] = atoi(currentWord.contents);
            if(j == g->adj.colEff-1){
                advNewline();
            }
            else{
                advWord();
            }
        }
    }
    g->jumlah_pesanan = atoi(currentWord.contents);
    for(i = 0; i < g->jumlah_pesanan; i++){
        advNewline();
        g->psn[i].t = atoi(currentWord.contents);
        advWord();
        g->psn[i].pickUp = currentWord.contents[0];
        advWord();
        g->psn[i].dropOff = currentWord.contents[0];
        advWord();
        if(currentWord.contents[0] == 'P'){
          char temp = currentWord.contents[0];
          advWord();
          int temp2 = atoi(currentWord.contents);
          createItem(&g->psn[i].tipeItem, temp, temp2);
        }else{
          createItem(&g->psn[i].tipeItem, currentWord.contents[0], -1);
        }
        psnTerurut = transformToPrioQueue(*g);
    }
    // ! LINE CODE INI JANGAN DIHAPUS, TANPA INI LISTDIN TIDAK STABIL!
    int len = listDinLength(g->bangunan);


    advNewline();
    // Currents
    copyWordFromWord(currentWord, &currentConfigFile);
    advNewline();
    currentTime = atoi(currentWord.contents);
    advWord();
    currentMoney = atoi(currentWord.contents);
    advNewline();
    finishedPesanan = atoi(currentWord.contents);
    advWord();
    totalMoney = atoi(currentWord.contents);
    
    advNewline();
    currentLocation.tipeBangunan = currentWord.contents[0];
    advWord();
    currentLocation.koor.X = atoi(currentWord.contents);
    advWord();
    currentLocation.koor.Y = atoi(currentWord.contents);
    
    
    // TODOs
    advNewline();
    int todolength = atoi(currentWord.contents);
    for(i = 0; i < todolength; i++){
        Pesanan tempP;
        advNewline();
        tempP.t = atoi(currentWord.contents);
        advWord();
        tempP.pickUp = currentWord.contents[0];
        advWord();
        tempP.dropOff = currentWord.contents[0];
        advWord();
        if(currentWord.contents[0] == 'P'){
          char temp = currentWord.contents[0];
          advWord();
          int temp2 = atoi(currentWord.contents);
          createItem(&tempP.tipeItem, temp, temp2);
          
        }else{
          createItem(&tempP.tipeItem, currentWord.contents[0], -1);
        }
        insertFirst(&TODO, tempP);
    }
    

    // In Progress
    advNewline();
    int inprolength = atoi(currentWord.contents);
    for(int i = 0; i < inprolength; i++){
        Pesanan tempP;
        advNewline();
        tempP.t = atoi(currentWord.contents);
        advWord();
        tempP.pickUp = currentWord.contents[0];
        advWord();
        tempP.dropOff = currentWord.contents[0];
        advWord();
        if(currentWord.contents[0] == 'P'){
          char temp = currentWord.contents[0];
          advWord();
          int temp2 = atoi(currentWord.contents);
          createItem(&tempP.tipeItem, temp, temp2);
          advWord();
          int temp3 = atoi(currentWord.contents);
          tempP.tipeItem.expiry_now = temp3;
        }else{
          createItem(&tempP.tipeItem, currentWord.contents[0], -1);
        }
        insertLast(&inProgress, tempP);
    }


    // PsnTerurut

    advNewline();
    Pesanan temppp;
    while(!isEmpty(psnTerurut)){
        dequeue(&psnTerurut, &temppp);
    }
    int psnlength = atoi(currentWord.contents);
    for(int i = 0; i < psnlength; i++){
        Pesanan tempP;
        advNewline();
        tempP.t = atoi(currentWord.contents);
        advWord();
        tempP.pickUp = currentWord.contents[0];
        advWord();
        tempP.dropOff = currentWord.contents[0];
        advWord();
        if(currentWord.contents[0] == 'P'){
          char temp = currentWord.contents[0];
          advWord();
          int temp2 = atoi(currentWord.contents);
          createItem(&tempP.tipeItem, temp, temp2);
          advWord();
          int temp3 = atoi(currentWord.contents);
          tempP.tipeItem.expiry_now = temp3;
        }else{
          createItem(&tempP.tipeItem, currentWord.contents[0], -1);
        }
        enqueue(&psnTerurut, tempP);
        // checker();
        // displayPesanan(TAIL(psnTerurut));
    }



    // Tas
    // idxTop, heavyItem
    advNewline();
    g->tas.idxTop = atoi(currentWord.contents);
    advWord();
    g->tas.heavyItem = atoi(currentWord.contents);

    for(int i = 0; i < inprolength; i++){
        Pesanan tempP;
        advNewline();
        tempP.t = atoi(currentWord.contents);
        advWord();
        tempP.pickUp = currentWord.contents[0];
        advWord();
        tempP.dropOff = currentWord.contents[0];
        advWord();
        if(currentWord.contents[0] == 'P'){
          char temp = currentWord.contents[0];
          advWord();
          int temp2 = atoi(currentWord.contents);
          createItem(&tempP.tipeItem, temp, temp2);
          advWord();
          int temp3 = atoi(currentWord.contents);
          tempP.tipeItem.expiry_now = temp3;
        }else{
          createItem(&tempP.tipeItem, currentWord.contents[0], -1);
        }
        g->tas.buffer[i] = tempP;
    }

    // Gadget List
    advNewline();
    for(int i = 0; i < 5; i++){
        int tempid = atoi(currentWord.contents);
        addGadget(&g->gl, Toko, tempid);
        if(i != 4){
            advNewline();
        }
    }

    // Ability
    advNewline();
    g->b.time = atoi(currentWord.contents);
    advNewline();
    g->b.freezeTime = atoi(currentWord.contents);
    advNewline();
    g->b.heavyItem = atoi(currentWord.contents);
    advNewline();
    g->b.returnToSender = atoi(currentWord.contents);
    advNewline();
    g->b.isSenterPengecilActivated = atoi(currentWord.contents);
}