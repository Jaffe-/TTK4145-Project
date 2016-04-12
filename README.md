TTK4145
=======


Bugs:
  * Hvis en heis er i etasje 0 (f.eks.), man gir internordre til en annen etasje og så trykker ekstern opp-knapp i etasje 0 _før_ floor signal event for etasje 1 skjer, så vil kostnaden regnes ut som 0 siden heisen teknisk sett fortsatt er i etasje 0.
  * Det har skjedd at løkken inne i SimulatedFSM::calculate har gått og gått uten å stoppe

Todo:
  * System for å holde oversikt over ordrer i systemet: Ideen er at vi _ikke_ sletter fra elevator_states når en heis detter ut, men heller flagger den som død/borte. Når heisen kobler til igjen får den siste ordreliste sånn at den kan fortsette med _interne_ ordrer. Eksterne ordrer håndteres av et system der eksterne knappetrykk genererer en id på formen "<IP>:<ID>" som alle heiser bruker som indeks i en map som holder alle aktive ordrer i systemet. Heisene regner ut hvem som skal ta ordren og registrerer dette. Når en heis har fullført ordren sender den en "Ferdig med ordre"-melding med ID-en til ordren, som gjør at ordren fjernes fra mapen hos alle. Hvis den som har tatt ordren detter ut av systemet gjør de resterende heisene en ny beregning, osv.
  * Lysene på eksterne knapper må implementeres
  * Muligens nye navn på Logic, ConnectionController og Sender?
  * Kommentere koden litt bedre
  * Figur(er)
