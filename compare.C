// Usage:  root -l 'compare.C("rock_only.root","rock_water.root",100000)'
void compare(const char* fA = "rock_only.root",
             const char* fB = "rock_water.root",
             double N = 1e5)
{
  TFile* a = TFile::Open(fA);
  TFile* b = TFile::Open(fB);
  TH1D* hA = (TH1D*)a->Get("Eback");
  TH1D* hB = (TH1D*)b->Get("Eback");

  double nA = hA->GetEntries(), nB = hB->GetEntries();
  printf("Backscattered per primary  rock only : %.5f +/- %.5f\n", nA/N, sqrt(nA)/N);
  printf("Backscattered per primary  rock+water: %.5f +/- %.5f\n", nB/N, sqrt(nB)/N);
  printf("Extra from water (B - A)             : %.5f +/- %.5f\n", (nB-nA)/N, sqrt(nA+nB)/N);

  hA->Sumw2(); hB->Sumw2();
  hA->Scale(1./N); hB->Scale(1./N);
  hA->SetLineColor(kBlue); hB->SetLineColor(kRed);
  hA->SetLineWidth(2);     hB->SetLineWidth(2);
  hA->SetTitle("Backscattered neutrons;Energy (MeV);neutrons per primary");
  hA->SetStats(0);

  TCanvas* c = new TCanvas("c", "Backscatter", 900, 600);
  c->SetLogx();
  c->SetLogy();
  c->SetGrid();
  hA->Draw("hist");
  hB->Draw("hist same");

  TLegend* leg = new TLegend(0.15, 0.75, 0.45, 0.88);
  leg->AddEntry(hA, "0.5 m rock", "l");
  leg->AddEntry(hB, "0.5 m rock + 0.5 m water", "l");
  leg->Draw();
  c->SaveAs("backscatter_comparison.png");
}
