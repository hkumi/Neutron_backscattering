// Track length of the neutrons that reach the scorers.
// Track length = total distance the neutron travelled from where it was
// created (the source, or an (n,2n) reaction) until it reached the scorer.
//
// Usage (in the build folder):
//   root -l 'tracklength.C("rock50_only.root","rock50_water.root",1000000)'
// Save only, no window:
//   root -l -b -q 'tracklength.C("rock50_only.root","rock50_water.root",1000000)'
//
// Saves (named after the second file):
//   tracklength_rock50_water.png / .pdf   3 plots: all backscattered,
//                                         thermal backscattered, transmitted
//   tracklength_rock50_water.txt          average track lengths

TString tlStem(const char* f)
{
  TString s = gSystem->BaseName(f);
  s.ReplaceAll(".root", "");
  return s;
}

// average track length (m) and number of neutrons for a selection
void tlMean(TTree* t, const char* cut, double& mean, double& n)
{
  t->SetEstimate(t->GetEntries() + 1);
  n = t->Draw("TrackLength_m", cut, "goff");
  mean = 0;
  if (n <= 0) return;
  double* v = t->GetV1();
  for (int i = 0; i < n; ++i) mean += v[i];
  mean /= n;
}

// histogram with log-spaced bins from 1 cm to 100 m
TH1D* tlHist(TTree* t, const char* name, const char* cut, double N)
{
  const int nb = 100;
  double edges[nb + 1];
  double lo = std::log10(0.01), hi = std::log10(100.);
  for (int i = 0; i <= nb; ++i) edges[i] = std::pow(10., lo + (hi - lo) * i / nb);
  TH1D* h = new TH1D(name, "", nb, edges);
  t->Project(name, "TrackLength_m", cut);
  h->Sumw2();
  h->Scale(1. / N);
  h->SetStats(0);
  h->SetLineWidth(2);
  return h;
}

void tracklength(const char* fA = "rock50_only.root",
                 const char* fB = "rock50_water.root",
                 double N = 1e6)
{
  TString labelA = tlStem(fA), labelB = tlStem(fB);
  TString outName = "tracklength_" + labelB;

  TFile* a = TFile::Open(fA);
  TFile* b = TFile::Open(fB);
  if (!a || !b) { printf("Could not open one of the files.\n"); return; }

  TTree* bA = (TTree*)a->Get("Backscattered");
  TTree* bB = (TTree*)b->Get("Backscattered");
  TTree* tA = (TTree*)a->Get("Transmitted");
  TTree* tB = (TTree*)b->Get("Transmitted");

  const char* thermal = "Energy_MeV < 5e-7";

  // ---------- average track lengths ----------
  struct Row { const char* label; TTree* ta; TTree* tb; const char* cut; };
  Row rows[3] = {
    {"Backscattered, all energies", bA, bB, ""},
    {"Backscattered, thermal",      bA, bB, thermal},
    {"Transmitted, all energies",   tA, tB, ""}
  };

  FILE* out = fopen((outName + ".txt").Data(), "w");
  printf("\nAverage track length (m)          A: %-18s  B: %s\n", labelA.Data(), labelB.Data());
  fprintf(out, "Average track length (m)          A: %-18s  B: %s\n", labelA.Data(), labelB.Data());
  for (int i = 0; i < 3; ++i) {
    double mA, nA, mB, nB;
    tlMean(rows[i].ta, rows[i].cut, mA, nA);
    tlMean(rows[i].tb, rows[i].cut, mB, nB);
    printf("%-32s  %6.2f m (%8.0f n)    %6.2f m (%8.0f n)\n", rows[i].label, mA, nA, mB, nB);
    fprintf(out, "%-32s  %6.2f m (%8.0f n)    %6.2f m (%8.0f n)\n", rows[i].label, mA, nA, mB, nB);
  }
  fclose(out);

  // ---------- plots ----------
  TH1D* h[3][2];
  const char* titles[3] = {
    "Backscattered neutrons, all energies;Track length (m);neutrons per primary",
    "Backscattered thermal neutrons (< 0.5 eV);Track length (m);neutrons per primary",
    "Transmitted neutrons;Track length (m);neutrons per primary"
  };
  h[0][0] = tlHist(bA, "hBackA", "",      N);  h[0][1] = tlHist(bB, "hBackB", "",      N);
  h[1][0] = tlHist(bA, "hThA",   thermal, N);  h[1][1] = tlHist(bB, "hThB",   thermal, N);
  h[2][0] = tlHist(tA, "hTrA",   "",      N);  h[2][1] = tlHist(tB, "hTrB",   "",      N);

  TCanvas* c = new TCanvas("c", "Track length", 1500, 500);
  c->Divide(3, 1);
  for (int i = 0; i < 3; ++i) {
    c->cd(i + 1);
    gPad->SetLogx(); gPad->SetLogy(); gPad->SetGrid();
    h[i][0]->SetLineColor(kBlue); h[i][1]->SetLineColor(kRed);
    h[i][0]->SetTitle(titles[i]);
    double mx = std::max(h[i][0]->GetMaximum(), h[i][1]->GetMaximum());
    h[i][0]->SetMaximum(mx * 100);      // room above the peak for the legend
    h[i][0]->SetMinimum(0.1 / N);
    h[i][0]->Draw("hist");
    h[i][1]->Draw("hist same");
    TLegend* leg = new TLegend(0.15, 0.78, 0.55, 0.88);
    leg->AddEntry(h[i][0], labelA, "l");
    leg->AddEntry(h[i][1], labelB, "l");
    leg->Draw();
  }
  c->SaveAs(outName + ".png");
  c->SaveAs(outName + ".pdf");
  printf("\nSaved: %s.png, %s.pdf, %s.txt\n", outName.Data(), outName.Data(), outName.Data());
}
