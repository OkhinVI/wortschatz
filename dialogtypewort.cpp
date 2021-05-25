#include "dialogtypewort.h"
#include "ui_dialogtypewort.h"

DialogTypeWort::DialogTypeWort(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTypeWort)
{
    ui->setupUi(this);
}

DialogTypeWort::~DialogTypeWort()
{
    delete ui;
}

WortDe::TypeWort DialogTypeWort::getTypeWort()
{
    if (ui->radioButton->isChecked())
        return WortDe::TypeWort::None;
    if (ui->radioButton_2->isChecked())  // сочитание слов  // Wortverbindung
        return WortDe::TypeWort::Combination;
    if (ui->radioButton_3->isChecked())  // существительное // das Nomen
        return WortDe::TypeWort::Noun;
    if (ui->radioButton_4->isChecked())  // глагол          // das Verb
        return WortDe::TypeWort::Verb;
    if (ui->radioButton_5->isChecked())  // прилагательное  // das Adjektiv
        return WortDe::TypeWort::Adjective;
    if (ui->radioButton_6->isChecked())  // местоимение     // das Pronomen
        return WortDe::TypeWort::Pronoun;
    if (ui->radioButton_7->isChecked())  // числительное    // das Numerale
        return WortDe::TypeWort::Numeral;
    if (ui->radioButton_8->isChecked())  // предлог         // die Präposition
        return WortDe::TypeWort::Pretext;
    if (ui->radioButton_9->isChecked())  // союз            // die Konjunktion
        return WortDe::TypeWort::Conjunction;
    if (ui->radioButton_10->isChecked()) // частица         // die Partikel
        return WortDe::TypeWort::Particle;
    if (ui->radioButton_11->isChecked()) // артикль         // der Artikel
        return WortDe::TypeWort::Artikel;
    if (ui->radioButton_12->isChecked()) // междометие      // die Interjektion
        return WortDe::TypeWort::Interjection;
    if (ui->radioButton_13->isChecked()) // наречие         // Adverb
        return WortDe::TypeWort::Adverb;

    return WortDe::TypeWort::None;
}

void DialogTypeWort::setTypeWort(WortDe::TypeWort tw)
{
    switch (tw) {
    case WortDe::TypeWort::None:
        ui->radioButton->setChecked(true);
        break;
    case WortDe::TypeWort::Combination:  // сочитание слов  // Wortverbindung
        ui->radioButton_2->setChecked(true);
        break;
    case WortDe::TypeWort::Noun:         // существительное // das Nomen
        ui->radioButton_3->setChecked(true);
        break;
    case WortDe::TypeWort::Verb:         // глагол          // das Verb
        ui->radioButton_4->setChecked(true);
        break;
    case WortDe::TypeWort::Adjective:    // прилагательное  // das Adjektiv
        ui->radioButton_5->setChecked(true);
        break;
    case WortDe::TypeWort::Pronoun:      // местоимение     // das Pronomen
        ui->radioButton_6->setChecked(true);
        break;
    case WortDe::TypeWort::Numeral:      // числительное    // das Numerale
        ui->radioButton_7->setChecked(true);
        break;
    case WortDe::TypeWort::Pretext:      // предлог         // die Präposition
        ui->radioButton_8->setChecked(true);
        break;
    case WortDe::TypeWort::Conjunction:  // союз            // die Konjunktion
        ui->radioButton_9->setChecked(true);
        break;
    case WortDe::TypeWort::Particle:     // частица         // die Partikel
        ui->radioButton_10->setChecked(true);
        break;
    case WortDe::TypeWort::Artikel:      // артикль         // der Artikel
        ui->radioButton_11->setChecked(true);
        break;
    case WortDe::TypeWort::Interjection: // междометие      // die Interjektion
        ui->radioButton_12->setChecked(true);
        break;
    case WortDe::TypeWort::Adverb:       // наречие         // Adverb
        ui->radioButton_13->setChecked(true);
        break;
    case WortDe::TypeWort::_last_one:
        break;
    }
}
