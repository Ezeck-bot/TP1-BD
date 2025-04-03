#include "entrypoint.h"
#include "sqlite3.h"
#include "knob.h"
#include "initialisation.h"

#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>

#define LOG_SQLITE3_ERROR(db) knob_log(KNOB_ERROR, "%s:%d: SQLITE3 ERROR: %s\n", __FILE__, __LINE__, sqlite3_errmsg(db))

//---- EXPLORER LES LIEUX
void see_lieu(sqlite3* db){

    knob_log(KNOB_INFO, "LISTE DES LIEUX : \n 1 pour LA MINE HANTEE \n 2 pour LE VILLAGE ABANDONNEE \n 3 pour LE CIMETIERE \n 4 pour LA MAISON DU JOUEUR");

    int lieu_saisis;
    scanf("%d", &lieu_saisis);

    sqlite3_stmt* stmt = NULL;
    char content[] =
    "SELECT\n" 
    "lieux.nom AS nom_lieu,\n"
    "lieux.description AS description_lieu,\n"
    "pnj.nom AS nom_pnj,\n"
    "pnj.description AS description_pnj,\n"
    "(SELECT GROUP_CONCAT(nom, ', ') FROM (SELECT DISTINCT nom FROM ennemis WHERE ennemis.id_lieu = lieux.id_lieu)) AS ennemis,\n" 
    "(SELECT GROUP_CONCAT(nom, ', ') FROM (SELECT DISTINCT nom FROM objets WHERE objets.id_lieu = lieux.id_lieu)) AS objets\n"
    "FROM lieux,pnj\n"
    "WHERE pnj.id_lieu = lieux.id_lieu AND lieux.id_lieu = ?;";

    int ret = sqlite3_prepare_v2(db,content,-1,&stmt,NULL);

    if(ret != SQLITE_OK){
        LOG_SQLITE3_ERROR(db);
    }

    if(sqlite3_bind_int(stmt,1,lieu_saisis) != SQLITE_OK){
        LOG_SQLITE3_ERROR(db);
    }
    
    for(int ret = sqlite3_step(stmt);ret != SQLITE_DONE;ret = sqlite3_step(stmt)){
        if(ret != SQLITE_ROW){
            LOG_SQLITE3_ERROR(db);
        }
        int column = 0;
        const unsigned char* nom_lieu = sqlite3_column_text(stmt,column++);
        const unsigned char* description_lieu = sqlite3_column_text(stmt,column++);
        const unsigned char* nom_pnj = sqlite3_column_text(stmt,column++);
        const unsigned char* description_pnj = sqlite3_column_text(stmt,column++);
        const unsigned char* ennemi = sqlite3_column_text(stmt,column++);
        const unsigned char* objets = sqlite3_column_text(stmt,column++);
        knob_log(KNOB_INFO,"LIEU : \n%s\n\n DESCRIPTION : \n%s\n\n ENNEMIS : \n%s\n\n OBJETS : \n%s\n\n PNJ : \n%s\n\n DESCRIPTION PNJ : \n%s\n ------------------------------------------------------------------------\n",nom_lieu,description_lieu,ennemi,objets,nom_pnj,description_pnj);
    }
    sqlite3_finalize(stmt);

    // MAJ POSITION JOUEUR
    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
    {
        LOG_SQLITE3_ERROR(db);
        return;
    }
    sqlite3_stmt *stmt2 = NULL;

    char content2[] =
        "UPDATE joueurs SET position_id = ?\n"
        "WHERE id_joueur = ?;";

    int ret2 = sqlite3_prepare_v2(db, content2, -1, &stmt2, NULL);

    if(sqlite3_bind_int(stmt2,1,lieu_saisis) != SQLITE_OK){
        LOG_SQLITE3_ERROR(db);
    }

    if(sqlite3_bind_int(stmt2,2,1) != SQLITE_OK){
        LOG_SQLITE3_ERROR(db);
    }

    ret2 = sqlite3_step(stmt2);
    
    sqlite3_finalize(stmt2);

    if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
    {
        LOG_SQLITE3_ERROR(db);
    }

    //Savoir si la quête secondaire est faite
    sqlite3_stmt* stmt_quete_secondaire = NULL;
    char content_quete_secondaire[] =
    "SELECT quetes.est_complete AS est_complete_quete_secondaire FROM lieux,pnj,quetes\n"
    "WHERE lieux.id_lieu = ? \n"
    "AND quetes.est_quete_principale = 0\n"
    "AND pnj.id_lieu = lieux.id_lieu\n"
    "AND quetes.id_pnj = pnj.id_pnj;";

    int ret_quete_secondaire = sqlite3_prepare_v2(db,content_quete_secondaire,-1,&stmt_quete_secondaire,NULL);

    if(ret_quete_secondaire != SQLITE_OK){
        LOG_SQLITE3_ERROR(db);
    }

    if(sqlite3_bind_int(stmt_quete_secondaire,1,lieu_saisis) != SQLITE_OK){
        LOG_SQLITE3_ERROR(db);
    }
    
    int column = 0;
    int est_complete_quete_secondaire;
    for(int ret_quete_secondaire = sqlite3_step(stmt_quete_secondaire);ret_quete_secondaire != SQLITE_DONE;ret_quete_secondaire = sqlite3_step(stmt_quete_secondaire)){
        if(ret_quete_secondaire != SQLITE_ROW){
            LOG_SQLITE3_ERROR(db);
        }
        int column = 0;
        est_complete_quete_secondaire = sqlite3_column_int(stmt_quete_secondaire,column++);

        if (est_complete_quete_secondaire == 1)
        {
            knob_log(KNOB_INFO,"QUETE SECONDAIRE : COMPLETE\n\n");
        } else {
            knob_log(KNOB_INFO,"QUETE SECONDAIRE : INCOMPLETE\n\n");
        }
        
    }
    sqlite3_finalize(stmt_quete_secondaire);

    //Savoir si la quête principale est faite
    sqlite3_stmt* stmt_quete_principale = NULL;
    char content_quete_principale[] =
    "SELECT quetes.est_complete AS est_complete_quete_principale, quetes.id_pnj AS id_pnj FROM lieux,pnj,quetes\n"
    "WHERE lieux.id_lieu = ? \n"
    "AND quetes.est_quete_principale = 1\n"
    "AND pnj.id_lieu = lieux.id_lieu\n"
    "AND quetes.id_pnj = pnj.id_pnj;";

    int ret_quete_principale = sqlite3_prepare_v2(db,content_quete_principale,-1,&stmt_quete_principale,NULL);

    if(ret_quete_principale != SQLITE_OK){
        LOG_SQLITE3_ERROR(db);
    }

    if(sqlite3_bind_int(stmt_quete_principale,1,lieu_saisis) != SQLITE_OK){
        LOG_SQLITE3_ERROR(db);
    }
    
    int id_pnj;
    int est_complete_quete_principale;
    for(int ret_quete_principale = sqlite3_step(stmt_quete_principale);ret_quete_principale != SQLITE_DONE;ret_quete_principale = sqlite3_step(stmt_quete_principale)){
        if(ret_quete_principale != SQLITE_ROW){
            LOG_SQLITE3_ERROR(db);
        }
        int column = 0;
        est_complete_quete_principale = sqlite3_column_int(stmt_quete_principale,column++);
        id_pnj = sqlite3_column_int(stmt_quete_principale,column++);

        if (est_complete_quete_principale == 1)
        {
            knob_log(KNOB_INFO,"QUETE PRINCIPALE : COMPLETE\n\n");
        } else {
            knob_log(KNOB_INFO,"QUETE PRINCIPALE : INCOMPLETE\n\n");
        }
        
    }
    sqlite3_finalize(stmt_quete_principale);

    //ramasser un objet ou faire les quêtes
    knob_log(KNOB_INFO,"VOULEZ VOUS RAMASSER UN OBJET OU FAIRE LES QUETES - 1 pour RAMASSER - 2 pour FAIRE LES QUETES\n");
    int faire_quete_ou_ramasser;
    scanf("%d", &faire_quete_ou_ramasser);
    if (faire_quete_ou_ramasser == 1)
    {
        //LISTE DES OBJETS
        sqlite3_stmt* stmt2 = NULL;
        char content2[] =
        "SELECT\n" 
        "id_objet,\n"
        "nom,\n"
        "description\n"
        "FROM objets\n"
        "WHERE id_lieu = ?;";
    
        int ret2 = sqlite3_prepare_v2(db,content2,-1,&stmt2,NULL);
    
        if(ret2 != SQLITE_OK){
            LOG_SQLITE3_ERROR(db);
        }
    
        if(sqlite3_bind_int(stmt2,1,lieu_saisis) != SQLITE_OK){
            LOG_SQLITE3_ERROR(db);
        }
    
        knob_log(KNOB_INFO, "OBJET DANS CE LIEU :\n");
        for(int ret2 = sqlite3_step(stmt2);ret2 != SQLITE_DONE;ret2 = sqlite3_step(stmt2)){
            if(ret2 != SQLITE_ROW){
                LOG_SQLITE3_ERROR(db);
            }
            int column = 0;
            int id_objet = sqlite3_column_int(stmt2,column++);
            const unsigned char* nom = sqlite3_column_text(stmt2,column++);
            const unsigned char* description = sqlite3_column_text(stmt2,column++);
            knob_log(KNOB_INFO,"NUMERO %d : %s -  DESCRIPTION : %s\n",id_objet,nom,description);
        }
    
        sqlite3_finalize(stmt2);
    
        knob_log(KNOB_INFO, "VEUILLEZ SAISIR LE NUMERO DE L'OBJET A RAMASSE");
    
        int objet_selectionne;
        scanf("%d", &objet_selectionne);

        //SAVOIR SI L'OBJET EST DEJA DANS L'INVENTAIRE
        sqlite3_stmt* stmt_nombre_inventaire = NULL;
        char content_nombre_inventaire[] =
        "SELECT count(*) AS nombre_objet FROM inventaire WHERE id_objet = 1;";
        int ret_nombre_inventaire = sqlite3_prepare_v2(db,content_nombre_inventaire,-1,&stmt_nombre_inventaire,NULL);
    
        if(ret_nombre_inventaire != SQLITE_OK){
            LOG_SQLITE3_ERROR(db);
        }
    
        ret_nombre_inventaire = sqlite3_step(stmt_nombre_inventaire);
        
        if(ret_nombre_inventaire != SQLITE_ROW){
            LOG_SQLITE3_ERROR(db);
        }

        int column = 0;
        int nombre_objet = sqlite3_column_int(stmt_nombre_inventaire,column);
        sqlite3_finalize(stmt_nombre_inventaire);

        if (nombre_objet > 0)
        {
            knob_log(KNOB_INFO, "\n----------------------------------------------------------------\nOBJET DEJA DANS L'INVENTAIRE L'INVENTAIRE\n----------------------------------------------------------------------\n");
        } else {
            //INSERTION DANS INVERTAIRE
            if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
            {
                LOG_SQLITE3_ERROR(db);
                return;
            }
            sqlite3_stmt *stmt3 = NULL;
        
            char content3[] =
                "INSERT INTO inventaire(id_joueur,id_objet)\n"
                "VALUES\n"
                "(?,?);";
            
            int ret3 = sqlite3_prepare_v2(db, content3, -1, &stmt3, NULL);
        
            if(sqlite3_bind_int(stmt3,1,1) != SQLITE_OK){
                LOG_SQLITE3_ERROR(db);
            }
            if(sqlite3_bind_int(stmt3,2,objet_selectionne) != SQLITE_OK){
                LOG_SQLITE3_ERROR(db);
            }
        
            if (sqlite3_step(stmt3) != SQLITE_DONE)
            {
                LOG_SQLITE3_ERROR(db);
            }
            
            sqlite3_finalize(stmt3);
        
            if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
            {
                LOG_SQLITE3_ERROR(db);
            }
        
            knob_log(KNOB_INFO, "\n----------------------------------------------------------------\nOBJET AJOUTEE AVEC SUCCES A L'INVENTAIRE\n----------------------------------------------------------------------\n");
        }
    } else if(faire_quete_ou_ramasser == 2) {
        knob_log(KNOB_INFO, "1 pour QUETE PRINCIPAL - 2 pour QUETE SECONDAIRE");
        int choix_quete;
        scanf("%d", &choix_quete);

        //Quête secondaire
        if (choix_quete == 1)
        {
            if (est_complete_quete_principale == 0)
            {
                //On lui demande s'il veut faire la quête principale
            knob_log(KNOB_INFO, "VOULEZ-VOUS FAIRE LA QUETE PRINCIPALE \n 1 pour Oui \n \n 0 pour Quitter");
            int faire_quete_principale;
            scanf("%d", &faire_quete_principale);
            if (faire_quete_principale == 1)
            {
                if (lieu_saisis == 1)
                {
                    //on vérifie si le joueur à une pioche dans son inventaire pour la quête 1
                    sqlite3_stmt* stmt_verification_pioche = NULL;
                    char content_verification_pioche[] =
                    "SELECT count(*) AS verification_pioche FROM objets,inventaire WHERE objets.id_objet = inventaire.id_objet AND objets.nom LIKE '%pioche%';";
                    int ret_verification_pioche = sqlite3_prepare_v2(db,content_verification_pioche,-1,&stmt_verification_pioche,NULL);

                    if(ret_verification_pioche != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }

                    ret_verification_pioche = sqlite3_step(stmt_verification_pioche);
                    
                    if(ret_verification_pioche != SQLITE_ROW){
                        LOG_SQLITE3_ERROR(db);
                    }
                    int column = 0;
                    int verification_pioche = sqlite3_column_int(stmt_verification_pioche,column);
                    sqlite3_finalize(stmt_verification_pioche);

                    if (verification_pioche == 0)
                    {
                        knob_log(KNOB_INFO, "IMPOSSIBLE DE FAIRE LA QUETE. OBJET PIOCHE REQUIS\n");
                        return;
                    }
                }
                else if (lieu_saisis == 2)
                {
                    //on vérifie si le joueur à une Magie de résistance au feu dans son inventaire pour la quête 1
                    sqlite3_stmt* stmt_verification_pioche = NULL;
                    char content_verification_pioche[] =
                    "SELECT count(*) AS verification_pioche FROM objets,inventaire WHERE objets.id_objet = inventaire.id_objet AND objets.nom LIKE '%Magie de résistance au feu%';";
                    int ret_verification_pioche = sqlite3_prepare_v2(db,content_verification_pioche,-1,&stmt_verification_pioche,NULL);

                    if(ret_verification_pioche != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }

                    ret_verification_pioche = sqlite3_step(stmt_verification_pioche);
                    
                    if(ret_verification_pioche != SQLITE_ROW){
                        LOG_SQLITE3_ERROR(db);
                    }
                    int column = 0;
                    int verification_pioche = sqlite3_column_int(stmt_verification_pioche,column);
                    sqlite3_finalize(stmt_verification_pioche);

                    if (verification_pioche == 0)
                    {
                        knob_log(KNOB_INFO, "IMPOSSIBLE DE FAIRE LA QUETE. OBJET MAGIE DE RESISTANCE AU FEU REQUIS\n");
                        return;
                    }
                }
                

                //il a 50% de rencontrer un enemie
                int rencontrer_enemie = rand() % 100;
                if (rencontrer_enemie <= 50)
                {
                    //un enemie apparaît
                    //récuperer la vie du joueur et sa force
                    sqlite3_stmt* stmt_joueur = NULL;
                    char content_joueur[] =
                    "SELECT vie,force FROM joueurs\n"
                    "WHERE id_joueur = 1;";

                    int ret_joueur = sqlite3_prepare_v2(db,content_joueur,-1,&stmt_joueur,NULL);

                    if(ret_joueur != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }
                    
                    int vie_joueur;
                    int force_joueur;
                    for(int ret_joueur = sqlite3_step(stmt_joueur);ret_joueur != SQLITE_DONE;ret_joueur = sqlite3_step(stmt_joueur)){
                        if(ret_joueur != SQLITE_ROW){
                            LOG_SQLITE3_ERROR(db);
                        }
                        int column = 0;
                        vie_joueur = sqlite3_column_int(stmt_joueur,column++);
                        force_joueur = sqlite3_column_int(stmt_joueur,column++);
                    }
                    sqlite3_finalize(stmt_joueur);

                    int vie_pour_experience = vie_joueur;
                    int force_pour_experience = force_joueur;

                    //récuperer la vie de l'enemie et sa force
                    sqlite3_stmt* stmt_ennemis = NULL;
                    char content_ennemis[] =
                    "SELECT nom,vie,force FROM ennemis\n"
                    "WHERE id_lieu = ? AND quete_principal_ou_secondaire = 1;";

                    int ret_ennemis = sqlite3_prepare_v2(db,content_ennemis,-1,&stmt_ennemis,NULL);

                    if(ret_ennemis != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }

                    if(sqlite3_bind_int(stmt_ennemis,1,lieu_saisis) != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }
                    
                    const unsigned char* nom;
                    int vie_ennemis;
                    int force_ennemis;
                    for(int ret_ennemis = sqlite3_step(stmt_ennemis);ret_ennemis != SQLITE_DONE;ret_ennemis = sqlite3_step(stmt_ennemis)){
                        if(ret_ennemis != SQLITE_ROW){
                            LOG_SQLITE3_ERROR(db);
                        }
                        int column = 0;
                        nom = knob_temp_strdup(sqlite3_column_text(stmt_ennemis,column++));
                        vie_ennemis = sqlite3_column_int(stmt_ennemis,column++);
                        force_ennemis = sqlite3_column_int(stmt_ennemis,column++);
                    }
                    sqlite3_finalize(stmt_ennemis);

                    knob_log(KNOB_INFO,"\n-------------------------------------------\nVOUS AVEZ RENCONTRE UN ENNEMI, NOM ENNEMI : %s VIE : %d, FORCE : %d\n",nom,vie_ennemis,force_ennemis);

                    knob_log(KNOB_INFO,"\nVOTRE VIE : %d, VOTRE FORCE ACTUELLE : %d\n",vie_joueur,force_joueur);

                    knob_log(KNOB_INFO,"\nDEBUT DU COMBAT");

                    while (vie_joueur > 0 && vie_ennemis > 0)
                    {
                        knob_log(KNOB_INFO,"\nVIE ENNEMIE : %d, FORCE : %d\n",vie_ennemis,force_ennemis);

                        knob_log(KNOB_INFO,"\nVOTRE VIE : %d, VOTRE FORCE ACTUELLE : %d\n",vie_joueur,force_joueur);

                        knob_log(KNOB_INFO, "Appuyer 1 pour attaquer, 2 pour defendre");
                        int choix_combat;
                        scanf("%d", &choix_combat);

                        if (choix_combat == 1)
                        {
                            int attaque_joueur = rand() % force_joueur;
                            vie_ennemis = vie_ennemis - attaque_joueur;

                            int attaque_ennemis = rand() % force_ennemis;
                            vie_joueur = vie_joueur - attaque_ennemis;
                        }
                        else if (choix_combat == 2) {
                            int attaque_ennemis = (rand() % force_ennemis)/2;
                            vie_ennemis = vie_ennemis - attaque_ennemis;
                        }
                    }
                    //si la vie du joueur est à 0 c'est GAME OVER il recommence le jeu
                    //s'il gagne il obtiens de l'expérience aléatoire

                    if (vie_joueur <= 0)
                    {
                        knob_log(KNOB_INFO, "GAME OVER");
                    } else {
                        //ADD EXPERIENCE
                        int ajout_vie = rand() % 30;
                        vie_pour_experience = vie_pour_experience + ajout_vie;

                        int ajout_force = rand() % 30;
                        force_pour_experience = force_pour_experience + ajout_force;
                        if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
                        {
                            LOG_SQLITE3_ERROR(db);
                            return;
                        }
                        sqlite3_stmt *stmt_update_joueur = NULL;

                        char content_update_joueur[] =
                            "UPDATE joueurs SET vie = ?, force = ?\n"
                            "WHERE id_joueur = 1;";

                        int ret_update_joueur = sqlite3_prepare_v2(db, content_update_joueur, -1, &stmt_update_joueur, NULL);

                        if(sqlite3_bind_int(stmt_update_joueur,1,vie_pour_experience) != SQLITE_OK){
                            LOG_SQLITE3_ERROR(db);
                        }

                        if(sqlite3_bind_int(stmt_update_joueur,2,force_pour_experience) != SQLITE_OK){
                            LOG_SQLITE3_ERROR(db);
                        }

                        ret_update_joueur = sqlite3_step(stmt_update_joueur);
                        
                        sqlite3_finalize(stmt_update_joueur);

                        if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
                        {
                            LOG_SQLITE3_ERROR(db);
                        }

                        // MAJ QUETE JOUEUR
                        if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
                        {
                            LOG_SQLITE3_ERROR(db);
                            return;
                        }
                        sqlite3_stmt *stmt2 = NULL;

                        char content2[] =
                            "UPDATE quetes SET est_complete = 1\n"
                            "WHERE id_pnj = ? AND est_quete_principale = 1;";

                        int ret2 = sqlite3_prepare_v2(db, content2, -1, &stmt2, NULL);

                        if(sqlite3_bind_int(stmt2,1,id_pnj) != SQLITE_OK){
                            LOG_SQLITE3_ERROR(db);
                        }

                        ret2 = sqlite3_step(stmt2);
                        
                        sqlite3_finalize(stmt2);

                        if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
                        {
                            LOG_SQLITE3_ERROR(db);
                        }

                        knob_log(KNOB_INFO,"\n\n ************\nQuête principale completée avec succès.\n************\n \n\n");
                    }
                    
                } else {
                    // MAJ QUETE JOUEUR
                    if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
                    {
                        LOG_SQLITE3_ERROR(db);
                        return;
                    }
                    sqlite3_stmt *stmt2 = NULL;

                    char content2[] =
                        "UPDATE quetes SET est_complete = 1\n"
                        "WHERE id_pnj = ? AND est_quete_principale = 1;";

                    int ret2 = sqlite3_prepare_v2(db, content2, -1, &stmt2, NULL);

                    if(sqlite3_bind_int(stmt2,1,id_pnj) != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }

                    ret2 = sqlite3_step(stmt2);
                    
                    sqlite3_finalize(stmt2);

                    if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
                    {
                        LOG_SQLITE3_ERROR(db);
                    }

                    knob_log(KNOB_INFO,"\n\n ************\nQuête principale completée avec succès.\n************\n \n\n");
                }
            }
            
            } else {
                knob_log(KNOB_INFO, "QUETE DEJA COMPLETE\n");
            }
        } 
        //Quête secondaire
        else if (choix_quete == 2) {
            if (est_complete_quete_secondaire == 0)
            {
                //Demander s'il veut parler au pnj
                knob_log(KNOB_INFO, "VOULEZ-VOUS PARLER AU PNJ :  \n 1 pour Oui \n \n 0 pour Quitter");
                int faire_quete_secondaire;
                scanf("%d", &faire_quete_secondaire);

                if (faire_quete_secondaire == 1)
                {
                    sqlite3_stmt* stmt3 = NULL;
                    char content[] =
                    "SELECT "
                    "pnj.id_pnj AS id_pnj, pnj.nom AS nom, pnj.description AS description, pnj.dialogue AS dialogue, pnj.dialogue_quete_accepte AS dialogue_quete_accepte, quetes.est_complete AS complete "
                    "FROM\n"
                    "pnj,quetes WHERE pnj.id_lieu = ? AND quetes.est_quete_principale = 0 AND pnj.id_pnj = quetes.id_pnj;";

                    int ret = sqlite3_prepare_v2(db,content,-1,&stmt3,NULL);

                    if(ret != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }

                    if(sqlite3_bind_int(stmt3,1,lieu_saisis) != SQLITE_OK){
                        LOG_SQLITE3_ERROR(db);
                    }

                    int est_complete;
                    int id_pnj;
                    const unsigned char* dialogue_quete_accepte;
                    for(int ret = sqlite3_step(stmt3);ret != SQLITE_DONE;ret = sqlite3_step(stmt3)){
                        if(ret != SQLITE_ROW){
                            LOG_SQLITE3_ERROR(db);
                        }
                        int column = 0;
                        id_pnj = sqlite3_column_int(stmt3,column++);
                        const unsigned char* nom_pnj = sqlite3_column_text(stmt3,column++);
                        const unsigned char* description_pnj = sqlite3_column_text(stmt3,column++);
                        const unsigned char* dialogue = sqlite3_column_text(stmt3,column++);
                        dialogue_quete_accepte = knob_temp_strdup(sqlite3_column_text(stmt3,column++));
                        est_complete = sqlite3_column_int(stmt3,column++);
                        
                        knob_log(KNOB_INFO,"\n\n PNJ : \n%s\n\n DESCRIPTION PNJ : \n%s\n\n DIALOGUE : \n%s\n ------------------------------------------------------------------------\n",nom_pnj,description_pnj,dialogue);
                    }
                    sqlite3_finalize(stmt3);

                    // ---
                    knob_log(KNOB_INFO, "\n 1 pour accepter la quete du pnj \n 2 pour refuser la quete du pnj");
                    int accepter_quete;
                    scanf("%d", &accepter_quete);

                    if (accepter_quete == 1)
                    {
                        
                        knob_log(KNOB_INFO,"\n\n Le PNJ VOUS DIT EN PLUS : %s\n",dialogue_quete_accepte);

                        //il a 50% de rencontrer un enemie
                        int rencontrer_enemie = rand() % 100;
                        if (rencontrer_enemie <= 50)
                        {
                            //un enemie apparaît
                            //récuperer la vie du joueur et sa force
                            sqlite3_stmt* stmt_joueur = NULL;
                            char content_joueur[] =
                            "SELECT vie,force FROM joueurs\n"
                            "WHERE id_joueur = 1;";

                            int ret_joueur = sqlite3_prepare_v2(db,content_joueur,-1,&stmt_joueur,NULL);

                            if(ret_joueur != SQLITE_OK){
                                LOG_SQLITE3_ERROR(db);
                            }
                            
                            int vie_joueur;
                            int force_joueur;
                            for(int ret_joueur = sqlite3_step(stmt_joueur);ret_joueur != SQLITE_DONE;ret_joueur = sqlite3_step(stmt_joueur)){
                                if(ret_joueur != SQLITE_ROW){
                                    LOG_SQLITE3_ERROR(db);
                                }
                                int column = 0;
                                vie_joueur = sqlite3_column_int(stmt_joueur,column++);
                                force_joueur = sqlite3_column_int(stmt_joueur,column++);
                            }

                            int vie_pour_experience = vie_joueur;
                            int force_pour_experience = force_joueur;

                            sqlite3_finalize(stmt_joueur);

                            //récuperer la vie du de l'enemie et sa force
                            sqlite3_stmt* stmt_ennemis = NULL;
                            char content_ennemis[] =
                            "SELECT nom,vie,force FROM ennemis\n"
                            "WHERE id_lieu = ? AND quete_principal_ou_secondaire = 0;";

                            int ret_ennemis = sqlite3_prepare_v2(db,content_ennemis,-1,&stmt_ennemis,NULL);

                            if(ret_ennemis != SQLITE_OK){
                                LOG_SQLITE3_ERROR(db);
                            }

                            if(sqlite3_bind_int(stmt_ennemis,1,lieu_saisis) != SQLITE_OK){
                                LOG_SQLITE3_ERROR(db);
                            }
                            
                            const unsigned char* nom;
                            int vie_ennemis;
                            int force_ennemis;
                            for(int ret_ennemis = sqlite3_step(stmt_ennemis);ret_ennemis != SQLITE_DONE;ret_ennemis = sqlite3_step(stmt_ennemis)){
                                if(ret_ennemis != SQLITE_ROW){
                                    LOG_SQLITE3_ERROR(db);
                                }
                                int column = 0;
                                nom = knob_temp_strdup(sqlite3_column_text(stmt_ennemis,column++));
                                vie_ennemis = sqlite3_column_int(stmt_ennemis,column++);
                                force_ennemis = sqlite3_column_int(stmt_ennemis,column++);
                            }
                            sqlite3_finalize(stmt_ennemis);

                            knob_log(KNOB_INFO,"\n-------------------------------------------\nVOUS AVEZ RENCONTRE UN ENNEMI, NOM ENNEMI : %s VIE : %d, FORCE : %d\n",nom,vie_ennemis,force_ennemis);

                            knob_log(KNOB_INFO,"\nVOTRE VIE : %d, VOTRE FORCE ACTUELLE : %d\n",vie_joueur,force_joueur);

                            knob_log(KNOB_INFO,"\nDEBUT DU COMBAT");

                            while (vie_joueur > 0 && vie_ennemis > 0)
                            {
                                knob_log(KNOB_INFO,"\nVIE ENNEMIE : %d, FORCE : %d\n",vie_ennemis,force_ennemis);

                                knob_log(KNOB_INFO,"\nVOTRE VIE : %d, VOTRE FORCE ACTUELLE : %d\n",vie_joueur,force_joueur);

                                knob_log(KNOB_INFO, "Appuyer 1 pour attaquer, 2 pour defendre");
                                int choix_combat;
                                scanf("%d", &choix_combat);

                                if (choix_combat == 1)
                                {
                                    int attaque_joueur = rand() % force_joueur;
                                    vie_ennemis = vie_ennemis - attaque_joueur;

                                    int attaque_ennemis = rand() % force_ennemis;
                                    vie_joueur = vie_joueur - attaque_ennemis;
                                }
                                else if (choix_combat == 2) {
                                    int attaque_ennemis = (rand() % force_ennemis)/2;
                                    vie_ennemis = vie_ennemis - attaque_ennemis;
                                }
                            }
                            //si la vie du joueur est à 0 c'est GAME OVER il recommence le jeu
                            //s'il gagne il obtiens de l'expérience aléatoire

                            if (vie_joueur <= 0)
                            {
                                knob_log(KNOB_INFO, "GAME OVER");
                            } else {
                                //ADD EXPERIENCE
                                int ajout_vie = rand() % 30;
                                vie_pour_experience = vie_pour_experience + ajout_vie;

                                int ajout_force = rand() % 30;
                                force_pour_experience = force_pour_experience + ajout_force;
                                if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                    return;
                                }
                                sqlite3_stmt *stmt_update_joueur = NULL;

                                char content_update_joueur[] =
                                    "UPDATE joueurs SET vie = ?, force = ?\n"
                                    "WHERE id_joueur = 1;";

                                int ret_update_joueur = sqlite3_prepare_v2(db, content_update_joueur, -1, &stmt_update_joueur, NULL);

                                if(sqlite3_bind_int(stmt_update_joueur,1,vie_pour_experience) != SQLITE_OK){
                                    LOG_SQLITE3_ERROR(db);
                                }

                                if(sqlite3_bind_int(stmt_update_joueur,2,force_pour_experience) != SQLITE_OK){
                                    LOG_SQLITE3_ERROR(db);
                                }

                                ret_update_joueur = sqlite3_step(stmt_update_joueur);
                                
                                sqlite3_finalize(stmt_update_joueur);

                                if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                }

                                // MAJ QUETE JOUEUR
                                if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                    return;
                                }
                                sqlite3_stmt *stmt2 = NULL;

                                char content2[] =
                                    "UPDATE quetes SET est_complete = 1\n"
                                    "WHERE id_pnj = ? AND est_quete_principale = 0;";

                                int ret2 = sqlite3_prepare_v2(db, content2, -1, &stmt2, NULL);

                                if(sqlite3_bind_int(stmt2,1,id_pnj) != SQLITE_OK){
                                    LOG_SQLITE3_ERROR(db);
                                }

                                ret2 = sqlite3_step(stmt2);
                                
                                sqlite3_finalize(stmt2);

                                if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                }

                                knob_log(KNOB_INFO,"\n\n ************\nQuête secondaire completée avec succès.\n************\n \n\n");

                                //si la quête secondaire est faite on ajoute la récompense dans objet et inventaire
                                if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                    return;
                                }
                                sqlite3_stmt *stmt_recompense = NULL;

                                char content_recompense[] =
                                    "INSERT INTO objets(nom,description,id_lieu)\n"
                                    "VALUES\n"
                                    "(?,?,?);";

                                int ret_recompense = sqlite3_prepare_v2(db, content_recompense, -1, &stmt_recompense, NULL);

                                //--
                                if (id_pnj == 1)
                                {
                                    //ajouter pioche dans table objet et inventaire
                                    const char *nom_objet[] = {"Pioche"};
                                    const char *description_objet[] = {"Permet au joueur de miner."};
                                    const char *lieu_objet[] = {"1"};
                                    
                                    if (sqlite3_bind_text(stmt_recompense, 1, nom_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                    {
                                        LOG_SQLITE3_ERROR(db);
                                    }
                                    if (sqlite3_bind_text(stmt_recompense, 2, description_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                    {
                                        LOG_SQLITE3_ERROR(db);
                                    }
                                    if (sqlite3_bind_text(stmt_recompense, 3, lieu_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                    {
                                        LOG_SQLITE3_ERROR(db);
                                    }
                                } else if (id_pnj == 2)
                                {
                                    //ajouter magie de résistance au feu dans table objet et inventaire
                                    const char *nom_objet[] = {"Magie de résistance au feu"};
                                    const char *description_objet[] = {"Permet de résister au feu pendant 45 secondes."};
                                    const char *lieu_objet[] = {"2"};

                                    if (sqlite3_bind_text(stmt_recompense, 1, nom_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                    {
                                        LOG_SQLITE3_ERROR(db);
                                    }
                                    if (sqlite3_bind_text(stmt_recompense, 2, description_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                    {
                                        LOG_SQLITE3_ERROR(db);
                                    }
                                    if (sqlite3_bind_text(stmt_recompense, 3, lieu_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                    {
                                        LOG_SQLITE3_ERROR(db);
                                    }
                                } else if (id_pnj == 3)
                                {
                                    //ajouter potion de force dans table objet et inventaire
                                    const char *nom_objet[] = {"Potion de force"};
                                    const char *description_objet[] = {"Une potion qui donne double la force de son utilisateur pendant 30 secondes."};
                                    const char *lieu_objet[] = {"3"};

                                    if (sqlite3_bind_text(stmt_recompense, 1, nom_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                    {
                                        LOG_SQLITE3_ERROR(db);
                                    }
                                    if (sqlite3_bind_text(stmt_recompense, 2, description_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                    {
                                        LOG_SQLITE3_ERROR(db);
                                    }
                                    if (sqlite3_bind_text(stmt_recompense, 3, lieu_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                    {
                                        LOG_SQLITE3_ERROR(db);
                                    }
                                }

                                if (sqlite3_step(stmt_recompense) != SQLITE_DONE)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                }
                                    
                                sqlite3_reset(stmt_recompense);
                                sqlite3_clear_bindings(stmt_recompense);

                                ret_recompense = sqlite3_step(stmt_recompense);
                                
                                sqlite3_finalize(stmt_recompense);

                                if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                }

                                //récupérer idendentifaint objet qui vient d'être ajouté
                                sqlite3_stmt* stmt_recup_last_id_objet = NULL;
                                char content_recup_last_id_objet[] =
                                "SELECT id_objet FROM objets ORDER BY id_objet DESC LIMIT 1;";
                                int ret_recup_last_id_objet = sqlite3_prepare_v2(db,content_recup_last_id_objet,-1,&stmt_recup_last_id_objet,NULL);

                                if(ret_recup_last_id_objet != SQLITE_OK){
                                    LOG_SQLITE3_ERROR(db);
                                }

                                ret_recup_last_id_objet = sqlite3_step(stmt_recup_last_id_objet);
                                
                                if(ret_recup_last_id_objet != SQLITE_ROW){
                                    LOG_SQLITE3_ERROR(db);
                                }
                                int column = 0;
                                int id_objet = sqlite3_column_int(stmt_recup_last_id_objet,column);
                                sqlite3_finalize(stmt_recup_last_id_objet);

                                //insérer dans inventaire
                                if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                    return;
                                }
                                sqlite3_stmt *stmt_recompense_inventaire = NULL;

                                char content_recompense_inventaire[] =
                                    "INSERT INTO inventaire(id_joueur,id_objet)\n"
                                    "VALUES\n"
                                    "(?,?);";
                                
                                int ret_recompense_inventaire = sqlite3_prepare_v2(db, content_recompense_inventaire, -1, &stmt_recompense_inventaire, NULL);

                                if(sqlite3_bind_int(stmt_recompense_inventaire,1,1) != SQLITE_OK){
                                    LOG_SQLITE3_ERROR(db);
                                }
                                if(sqlite3_bind_int(stmt_recompense_inventaire,2,id_objet) != SQLITE_OK){
                                    LOG_SQLITE3_ERROR(db);
                                }

                                if (sqlite3_step(stmt_recompense_inventaire) != SQLITE_DONE)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                }
                                
                                sqlite3_finalize(stmt_recompense_inventaire);

                                if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                }
                                
                                knob_log(KNOB_INFO,"\n\n ************\nRécompense obtenue.\n************\n \n\n");
                            }
                            
                        } else {
                            // MAJ QUETE JOUEUR
                            if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
                            {
                                LOG_SQLITE3_ERROR(db);
                                return;
                            }
                            sqlite3_stmt *stmt2 = NULL;

                            char content2[] =
                                "UPDATE quetes SET est_complete = 1\n"
                                "WHERE id_pnj = ? AND est_quete_principale = 0;";

                            int ret2 = sqlite3_prepare_v2(db, content2, -1, &stmt2, NULL);

                            if(sqlite3_bind_int(stmt2,1,id_pnj) != SQLITE_OK){
                                LOG_SQLITE3_ERROR(db);
                            }

                            ret2 = sqlite3_step(stmt2);
                            
                            sqlite3_finalize(stmt2);

                            if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
                            {
                                LOG_SQLITE3_ERROR(db);
                            }

                            knob_log(KNOB_INFO,"\n\n ************\nQuête secondaire completée avec succès.\n************\n \n\n");

                            //si la quête secondaire est faite on ajoute la récompense dans objet et inventaire
                            if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
                            {
                                LOG_SQLITE3_ERROR(db);
                                return;
                            }
                            sqlite3_stmt *stmt_recompense = NULL;

                            char content_recompense[] =
                                "INSERT INTO objets(nom,description,id_lieu)\n"
                                "VALUES\n"
                                "(?,?,?);";

                            int ret_recompense = sqlite3_prepare_v2(db, content_recompense, -1, &stmt_recompense, NULL);

                            //--
                            if (id_pnj == 1)
                            {
                                //ajouter pioche dans table objet et inventaire
                                const char *nom_objet[] = {"Pioche"};
                                const char *description_objet[] = {"Permet au joueur de miner."};
                                const char *lieu_objet[] = {"1"};
                                
                                if (sqlite3_bind_text(stmt_recompense, 1, nom_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                }
                                if (sqlite3_bind_text(stmt_recompense, 2, description_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                }
                                if (sqlite3_bind_text(stmt_recompense, 3, lieu_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                }
                            } else if (id_pnj == 2)
                            {
                                //ajouter magie de résistance au feu dans table objet et inventaire
                                const char *nom_objet[] = {"Magie de résistance au feu"};
                                const char *description_objet[] = {"Permet de résister au feu pendant 45 secondes."};
                                const char *lieu_objet[] = {"2"};

                                if (sqlite3_bind_text(stmt_recompense, 1, nom_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                }
                                if (sqlite3_bind_text(stmt_recompense, 2, description_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                }
                                if (sqlite3_bind_text(stmt_recompense, 3, lieu_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                }
                            } else if (id_pnj == 3)
                            {
                                //ajouter potion de force dans table objet et inventaire
                                const char *nom_objet[] = {"Potion de force"};
                                const char *description_objet[] = {"Une potion qui donne double la force de son utilisateur pendant 30 secondes."};
                                const char *lieu_objet[] = {"3"};

                                if (sqlite3_bind_text(stmt_recompense, 1, nom_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                }
                                if (sqlite3_bind_text(stmt_recompense, 2, description_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                }
                                if (sqlite3_bind_text(stmt_recompense, 3, lieu_objet[0], -1, SQLITE_STATIC) != SQLITE_OK)
                                {
                                    LOG_SQLITE3_ERROR(db);
                                }
                            }

                            if (sqlite3_step(stmt_recompense) != SQLITE_DONE)
                            {
                                LOG_SQLITE3_ERROR(db);
                            }
                                
                            sqlite3_reset(stmt_recompense);
                            sqlite3_clear_bindings(stmt_recompense);

                            ret_recompense = sqlite3_step(stmt_recompense);
                            
                            sqlite3_finalize(stmt_recompense);

                            if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
                            {
                                LOG_SQLITE3_ERROR(db);
                            }

                            //récupérer idendentifaint objet qui vient d'être ajouté
                            sqlite3_stmt* stmt_recup_last_id_objet = NULL;
                            char content_recup_last_id_objet[] =
                            "SELECT id_objet FROM objets ORDER BY id_objet DESC LIMIT 1;";
                            int ret_recup_last_id_objet = sqlite3_prepare_v2(db,content_recup_last_id_objet,-1,&stmt_recup_last_id_objet,NULL);

                            if(ret_recup_last_id_objet != SQLITE_OK){
                                LOG_SQLITE3_ERROR(db);
                            }

                            ret_recup_last_id_objet = sqlite3_step(stmt_recup_last_id_objet);
                            
                            if(ret_recup_last_id_objet != SQLITE_ROW){
                                LOG_SQLITE3_ERROR(db);
                            }
                            int column = 0;
                            int id_objet = sqlite3_column_int(stmt_recup_last_id_objet,column);
                            sqlite3_finalize(stmt_recup_last_id_objet);

                            //insérer dans inventaire
                            if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
                            {
                                LOG_SQLITE3_ERROR(db);
                                return;
                            }
                            sqlite3_stmt *stmt_recompense_inventaire = NULL;

                            char content_recompense_inventaire[] =
                                "INSERT INTO inventaire(id_joueur,id_objet)\n"
                                "VALUES\n"
                                "(?,?);";
                            
                            int ret_recompense_inventaire = sqlite3_prepare_v2(db, content_recompense_inventaire, -1, &stmt_recompense_inventaire, NULL);

                            if(sqlite3_bind_int(stmt_recompense_inventaire,1,1) != SQLITE_OK){
                                LOG_SQLITE3_ERROR(db);
                            }
                            if(sqlite3_bind_int(stmt_recompense_inventaire,2,id_objet) != SQLITE_OK){
                                LOG_SQLITE3_ERROR(db);
                            }

                            if (sqlite3_step(stmt_recompense_inventaire) != SQLITE_DONE)
                            {
                                LOG_SQLITE3_ERROR(db);
                            }
                            
                            sqlite3_finalize(stmt_recompense_inventaire);

                            if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
                            {
                                LOG_SQLITE3_ERROR(db);
                            }
                            
                            knob_log(KNOB_INFO,"\n\n ************\nRécompense obtenue.\n************\n \n\n");
                        }
                        
                    }
                }
            
            } else {
                knob_log(KNOB_INFO, "QUETE DEJA COMPLETE\n");
            }
        }
    }
}

//INFO JOUEUR
char see_info_joueur(sqlite3* db){
    sqlite3_stmt* stmt = NULL;
    char content[] =
    "SELECT\n"
    "vie,force\n"
    "FROM joueurs\n"
    "WHERE\n"
    "id_joueur = 1;";

    int ret = sqlite3_prepare_v2(db,content,-1,&stmt,NULL);

    if(ret != SQLITE_OK){
        LOG_SQLITE3_ERROR(db);
    }

    int vie;
    int force;
    for(int ret = sqlite3_step(stmt);ret != SQLITE_DONE;ret = sqlite3_step(stmt)){
        if(ret != SQLITE_ROW){
            LOG_SQLITE3_ERROR(db);
        }
        int column = 0;
        vie = sqlite3_column_int(stmt,column++);
        force = sqlite3_column_int(stmt,column++);
    }
    return printf("Vie : %d - Force : %d \n\n", vie, force);
}

//--- AFFICHER L'INVENTAIRE
void see_inventaire(sqlite3* db){
    //SAVOIR SI L'INVENTAIR EST VIDE
    sqlite3_stmt* stmt_verif_inventaire = NULL;
    char content_verif_inventaire[] =
    "SELECT COUNT(*) AS nombre_inventaire\n"
    "FROM inventaire WHERE id_joueur = 1;";
    int ret_verif_inventaire = sqlite3_prepare_v2(db,content_verif_inventaire,-1,&stmt_verif_inventaire,NULL);

    if(ret_verif_inventaire != SQLITE_OK){
        LOG_SQLITE3_ERROR(db);
    }

    ret_verif_inventaire = sqlite3_step(stmt_verif_inventaire);
    
    if(ret_verif_inventaire != SQLITE_ROW){
        LOG_SQLITE3_ERROR(db);
    }
    int column = 0;
    int nombre_inventaire = sqlite3_column_int(stmt_verif_inventaire,column);
    sqlite3_finalize(stmt_verif_inventaire);

    if (nombre_inventaire > 0)
    {
        sqlite3_stmt* stmt = NULL;
        char content[] =
        "SELECT\n"
        "inventaire.id_inventaire AS inventaire,\n"
        "objets.nom AS nom,\n"
        "objets.description AS description\n"
        "FROM inventaire,objets\n"
        "WHERE\n"
        "inventaire.id_objet = objets.id_objet AND\n"
        "inventaire.id_joueur = 1;";

        int ret = sqlite3_prepare_v2(db,content,-1,&stmt,NULL);

        if(ret != SQLITE_OK){
            LOG_SQLITE3_ERROR(db);
        }

        for(int ret = sqlite3_step(stmt);ret != SQLITE_DONE;ret = sqlite3_step(stmt)){
            if(ret != SQLITE_ROW){
                LOG_SQLITE3_ERROR(db);
            }
            int column = 0;
            int id_inventaire = sqlite3_column_int(stmt,column++);
            const unsigned char* nom = sqlite3_column_text(stmt,column++);
            const unsigned char* description = sqlite3_column_text(stmt,column++);
            knob_log(KNOB_INFO,"NUMERO %d - %s, DESCRIPTION : %s\n",id_inventaire,nom,description);
        }
        knob_log(KNOB_INFO, "------------------------------------------------------------------------\n\n");

        sqlite3_finalize(stmt);
        
        knob_log(KNOB_INFO, "SAISIR LE NUMERO DE L'OBJET A SUPPRIMER DE L'INVENTAIRE OU APPUYER SUR 0 POUR QUITTER");

        int objet_a_supprimer;
        scanf("%d", &objet_a_supprimer);
        if (objet_a_supprimer != 0)
        {
            //DELETE OBJET
            if (sqlite3_exec(db, "BEGIN;", NULL, NULL, NULL) != SQLITE_OK)
            {
                LOG_SQLITE3_ERROR(db);
                return;
            }
            sqlite3_stmt *stmt2 = NULL;

            char content2[] =
                "DELETE FROM inventaire\n"
                "WHERE id_inventaire = ?;";

            int ret2 = sqlite3_prepare_v2(db, content2, -1, &stmt2, NULL);

            int joueur = 1;

            if(sqlite3_bind_int(stmt2,1,objet_a_supprimer) != SQLITE_OK){
                LOG_SQLITE3_ERROR(db);
            }

            if(sqlite3_bind_int(stmt2,2,joueur) != SQLITE_OK){
                LOG_SQLITE3_ERROR(db);
            }

            ret2 = sqlite3_step(stmt2);
            
            sqlite3_finalize(stmt2);

            if (sqlite3_exec(db, "COMMIT;", NULL, NULL, NULL) != SQLITE_OK)
            {
                LOG_SQLITE3_ERROR(db);
            }
            //FIN DELETE OBJET

            knob_log(KNOB_INFO, "\n-------------------OBJET SUPPRIME AVEC SUCCES DE L'INVENTAIRE-------------------\n");
        }
    } else {
        knob_log(KNOB_INFO, "********************* VOTRE INVENTAIRE EST VIDE *********************\n\n");
    }
}

void raylib_start(void){
    srand(time(NULL));

    int end = 0;
    while (!end)
    {
        knob_log(KNOB_INFO, "\n-------------------BIENVENUE DANS LAST OF KINGDOM-------------------\n");
        knob_log(KNOB_INFO, "1 POUR CONTINUER VOTRE PARTIE - AUTRE CHIFFRE POUR UNE NOUVELLE PARTIE");
        int partie;
        scanf("%d", &partie);
        if (partie == 1)
        {
            knob_log(KNOB_INFO, "Entrer votre nom de joueur");
            char nom[100];
            scanf("%s", nom);
            nom[strcspn(nom, "\n")] = 0;

            char nom_fichier[128];
            snprintf(nom_fichier, sizeof(nom_fichier), "aventure_quete_%s.db", nom);

            sqlite3* db = NULL;

            int db_exits = knob_file_exists(nom_fichier);
            if(db_exits == 1){
                if (sqlite3_open(nom_fichier, &db) != SQLITE_OK) {
                    knob_log(KNOB_INFO, "Erreur ouverture BD: %s\n", sqlite3_errmsg(db));
                    sqlite3_close(db);
                    return;
                }

                knob_log(KNOB_INFO, "BON RETOUR %s\n", nom);

                see_info_joueur(db);

                int end_menu = 0;
                while (!end_menu)
                {
                    knob_log(KNOB_INFO, "1 POUR EXPLORER DES LIEUX - 2 POUR VOIR VOTRE INVENTAIRE ");
                    int choix_action;
                    scanf("%d", &choix_action);
                    if (choix_action == 1)
                    {
                        see_lieu(db);
                    } else {
                        see_inventaire(db);
                    }
                }
                
            } else {
                knob_log(KNOB_INFO, "PARTIE INEXISTANTE");
            }
        } 
        //-----------NOUVEAU JOUEUR
        else {
            knob_log(KNOB_INFO, "Entrer votre nom de joueur");
            char nom[100];
            scanf("%s", nom);
            nom[strcspn(nom, "\n")] = 0;

            char nom_fichier[128];
            snprintf(nom_fichier, sizeof(nom_fichier), "aventure_quete_%s.db", nom);

            sqlite3* db = NULL;

            int db_exits = knob_file_exists(nom_fichier);
            if(db_exits == 1){
                knob_log(KNOB_INFO, "BASE DE DONNEE DEJA EXISTANTE");
            } else {
                if (sqlite3_open(nom_fichier, &db) != SQLITE_OK) {
                    knob_log(KNOB_INFO, "Erreur ouverture BD: %s\n", sqlite3_errmsg(db));
                    sqlite3_close(db);
                    return;
                }

                initialisation_db(db, nom);

                knob_log(KNOB_INFO, "Joueur : %s \n Vie : 100 \n Force : 100 \n Position : Maison du joueur\n\n--------",nom);
                knob_log(KNOB_INFO, "1 POUR EXPLORER DES LIEUX - 2 POUR VOIR VOTRE INVENTAIRE ");
                int choix_action;
                scanf("%d", &choix_action);
                if (choix_action == 1)
                {
                    see_lieu(db);
                } else {
                    see_inventaire(db);
                }
            }
        }
        
    }
    return;
}