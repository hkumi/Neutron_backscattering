// Usage:  root -l 'plot.C("rock_water.root")'
void plot(const char* f = "output0.root")
{
  TFile* file = TFile::Open(f);
  TH1D* hB = (TH1D*)file->Get("Eback");
  TH1D* hT = (TH1D*)file->Get("Etrans");

  TCanvas* c = new TCanvas("c", f, 1200, 500);
  c->Divide(2, 1);

  c->cd(1); gPad->SetLogx(); gPad->SetLogy(); gPad->SetGrid();
  hB->SetLineWidth(2); hB->Draw("hist");

  c->cd(2); gPad->SetLogx(); gPad->SetLogy(); gPad->SetGrid();
  hT->SetLineWidth(2); hT->Draw("hist");
}
