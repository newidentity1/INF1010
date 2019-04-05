/*
* Date : 25 f�vrier 2019
* Auteur : AbdeB
*/

#include "Restaurant.h"
#include "LectureFichierEnSections.h"
#include "debogageMemoire.hpp"
#include <cassert>


//constructeurs 
Restaurant::Restaurant() :
	Restaurant{"menu.txt", "Inconnu", TypeMenu::Matin}
{
}

Restaurant::Restaurant(const string& nomFichier, string_view nom, TypeMenu moment) :
	nom_{nom},
	momentJournee_{moment},
	chiffreAffaire_{0},
	menuMatin_{new GestionnairePlats{nomFichier, TypeMenu::Matin}},
	menuMidi_ {new GestionnairePlats{nomFichier, TypeMenu::Midi }},
	menuSoir_ {new GestionnairePlats{nomFichier, TypeMenu::Soir }},
	fraisLivraison_{},
    tables_{new GestionnaireTables}
{
	//lireTables(nomFichier);
    tables_->lireTables(nomFichier);
	lireAdresses(nomFichier);
}

// Destructeur.
Restaurant::~Restaurant()
{
	delete menuMatin_;
	delete menuMidi_;
	delete menuSoir_;
    delete tables_;
}


// Setters.

void Restaurant::setMoment(TypeMenu moment)
{
	momentJournee_ = moment; 
}

void Restaurant::setNom(string_view nom)
{
	nom_ = nom;
}

 void  Restaurant::setChiffreAffaire( double chiffre)
{ chiffreAffaire_ = chiffre;
}
// Getters.

string Restaurant::getNom() const
{
	return nom_; 
}

TypeMenu Restaurant::getMoment() const
{
	return momentJournee_; 
}

GestionnaireTables* Restaurant::getTables() const
{
    return tables_;
}

double Restaurant::getFraisLivraison(int index) const
{
	return fraisLivraison_[index];
}

// Autres methodes.

void Restaurant::libererTable(int id)
{
    Table* tableALiberer = tables_->getTable(id);
	if (tableALiberer != nullptr)
    {
		chiffreAffaire_ += tableALiberer->getChiffreAffaire();
        chiffreAffaire_ += calculerReduction(tableALiberer->getClientPrincipal(), tableALiberer->getChiffreAffaire(), id == ID_TABLE_LIVRAISON);
		tableALiberer->libererTable();
	}
}

ostream& operator<<(ostream& os, const Restaurant& restaurant)
{
   os << "Le restaurant " << restaurant.getNom();
   if (restaurant.chiffreAffaire_ != 0)
       os << " a fait un chiffre d'affaire de : " << restaurant.chiffreAffaire_ << "$" << endl;
   else
       os << " n'a pas fait de benefice ou le chiffre n'est pas encore calcule." << endl;

   os << "-Voici les tables : " << endl;

   restaurant.tables_->afficherTables(os);
   os << endl;
    
   os << "-Voici son menu : " << endl;
   for (TypeMenu typeMenu : { TypeMenu::Matin, TypeMenu::Midi, TypeMenu::Soir })
   {
       GestionnairePlats* menuResto = restaurant.getMenu(typeMenu);
       os << restaurant.getNomTypeMenu(typeMenu) << " : " << endl;
       restaurant.getMenu(typeMenu)->afficherPlats(os);
       os  << "Le plat le moins cher est : ";
       menuResto->trouverPlatMoinsCher()->afficherPlat(os);
       os << endl;
   }
    
	return os;
}

void Restaurant::commanderPlat(const string& nom, int idTable)
{
    if (Table* table = tables_->getTable(idTable); table && table->estOccupee())
		if (Plat* plat = menuActuel()->trouverPlat(nom))
        {
			table->commander(plat);
			return;
		}
	cout << "Erreur : table vide ou plat introuvable." << endl << endl;
}

bool Restaurant::operator <(const Restaurant& autre) const 
{
	return chiffreAffaire_ < autre.chiffreAffaire_;
}

bool Restaurant::placerClients(Client* client)
{
	const int tailleGroupe = client->getTailleGroupe();
	//TODO : trouver la table la plus adaptée pour le client.
    Table* meilleurTable = tables_->getMeilleureTable(tailleGroupe);
	//TODO : Si possible assigner la table au client sinon retourner false.
    if (meilleurTable != nullptr)
    {
    meilleurTable->placerClients(tailleGroupe);
    meilleurTable->setClientPrincipal(client);
    client->setTable(meilleurTable);
        return true;
    }
    else
    {
         return false;
    }
}

bool Restaurant::livrerClient(Client* client, const vector<string>& commande)
{
	if (dynamic_cast<ClientPrestige*>(client))
    {
		// TODO: Placer le client principal a la table fictive en utilisant la classe GestionnaireTables.
        Table* livraison = tables_->getTable(ID_TABLE_LIVRAISON);
        
        livraison->setClientPrincipal(client);
		
		// TODO: Placer client à la table fictive en utilisant la classe GestionnaireTables.
        livraison->placerClients(1);
		
		// Placer la commande
		for (unsigned int i = 0; i < commande.size(); i++)
			commanderPlat(commande[i], ID_TABLE_LIVRAISON);
		// Liberer la table fictive.
		libererTable(ID_TABLE_LIVRAISON);
		return true;
	}
	else
    {
		return false;
	}
}

double Restaurant::calculerReduction(Client* client, double montant, bool estLivraison)
{
    return client->getReduction(*this, montant, estLivraison);
}


GestionnairePlats* Restaurant::getMenu(TypeMenu typeMenu) const
{
	switch (typeMenu) {
	case TypeMenu::Matin : return menuMatin_;
	case TypeMenu::Midi  : return menuMidi_;
	case TypeMenu::Soir  : return menuSoir_;
	}
	assert(false && "Le type du menu est invalide");
	return nullptr;  // On ne devrait jamais se rendre � cette ligne.
}

GestionnairePlats* Restaurant::menuActuel() const
{
	return getMenu(momentJournee_);
}

void Restaurant::lireAdresses(const string& nomFichier)
{
	LectureFichierEnSections fichier{nomFichier};
	fichier.allerASection("-ADDRESSES");
	while (!fichier.estFinSection())
    {
		int zone; double frais;
		fichier >> zone >> frais;
		fraisLivraison_[zone] = frais;
	}
}

double Restaurant::getChiffreAffaire()
{
	return chiffreAffaire_;
}

string Restaurant::getNomTypeMenu(TypeMenu typeMenu) const
{
	return string{nomsDesTypesDeMenu[static_cast<int>(typeMenu)]};
}
