TTK4145
=======


Bugs:
  * Hvis en heis er i etasje 0 (f.eks.), man gir internordre til en annen etasje og så trykker ekstern opp-knapp i etasje 0 _før_ floor signal event for etasje 1 skjer, så vil kostnaden regnes ut som 0 siden heisen teknisk sett fortsatt er i etasje 0.
  * Det har skjedd at løkken inne i SimulatedFSM::calculate har gått og gått uten å stoppe

Todo:
  * System for å holde oversikt over ordrer i systemet
  * Muligens nye navn på Logic, ConnectionController og Sender?
  * Kommentere koden litt bedre
  * Figur(er)