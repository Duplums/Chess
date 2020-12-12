#ifndef TRANSPOSITIONTABLE_H
#define TRANSPOSITIONTABLE_H

#include<unordered_map>
#include<iostream>

// 1 Entry = 22o

// TODO:
// - Créer la table de transpo principale et définir les règles de remplacement
// - Créer la table de hash pour les évaluations de situation avec ses règles de remplacement
// - Créer la table de hash pour les pions spécialement (on hash seulement la position des pions)

// Taille admissible = 200 Mo.

// - Pour la table principale, 100Mo => ~4.5M entrées
// - Pour la table de hash des évaluations, 50 Mo => ~2.2M entrées
// - Pour la table de hash des pions, 50 Mo => ~2.2M entrées

class TranspositionTable
{
public:
    TranspositionTable();

private:
    std::unordered_map<int, int> m_table;
};

#endif // TRANSPOSITIONTABLE_H
