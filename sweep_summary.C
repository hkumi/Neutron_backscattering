// Usage (in the build folder, after sweep.sh):
//   root -l -b -q 'sweep_summary.C(1000000)'
// Reads rockXX_only.root and rockXX_water.root for every thickness and saves
//   sweep_summary.txt            table of all numbers (with sigma)
//   sweep_transmission.png/.pdf  transmitted neutrons vs rock thickness
//   sweep_thermal.png/.pdf       thermal backscattered neutrons vs rock thickness

void sweep_summary(double N = 1e6)
{
  const int n = 6;
  int cm[n] = {10, 20, 30, 50, 75, 100};

  const char* thermal = "Energy_MeV < 5e-7";      // thermal = below 0.5 eV

  TGraphErrors* gTransA = new TGraphErrors();
  TGraphErrors* gTransB = new TGraphErrors();
  TGraphErrors* gThermA = new TGraphErrors();
  TGraphErrors* gThermB = new TGraphErrors();

  FILE* out = fopen("sweep_summary.txt", "w");
  const char* head =
    "Rock  | Transmitted       Transmitted       | Thermal backscatter (< 0.5 eV)            | Water signal\n"
    "(cm)  | rock only         rock + water      | rock only   rock + water   B - A          | (%)   sigma\n"
    "------+-------------------------------------+-------------------------------------------+---------------\n";
  printf("\nNeutrons per run: %.0f   (all numbers are per primary neutron)\n\n", N);
  fprintf(out, "Neutrons per run: %.0f   (all numbers are per primary neutron)\n\n", N);
  printf("%s", head); fprintf(out, "%s", head);

  int k = 0;
  for (int i = 0; i < n; ++i) {
    TString fA = Form("rock%d_only.root",  cm[i]);
    TString fB = Form("rock%d_water.root", cm[i]);
    if (gSystem->AccessPathName(fA) || gSystem->AccessPathName(fB)) {
      printf("%4d  | files not found, skipped\n", cm[i]);
      continue;
    }
    TFile* a = TFile::Open(fA);
    TFile* b = TFile::Open(fB);
    TTree* bA = (TTree*)a->Get("Backscattered");
    TTree* bB = (TTree*)b->Get("Backscattered");
    TTree* tA = (TTree*)a->Get("Transmitted");
    TTree* tB = (TTree*)b->Get("Transmitted");

    double trA = tA->GetEntries(), trB = tB->GetEntries();
    double thA = bA->GetEntries(thermal), thB = bB->GetEntries(thermal);

    double diff  = (thB - thA) / N;
    double err   = sqrt(thA + thB) / N;
    double sigma = (err > 0) ? diff / err : 0;
    // percentage only makes sense if rock-only has some thermal neutrons
    TString perc = (thA > 0) ? Form("%+6.1f", 100. * (thB - thA) / thA) : TString("   n/a");

    const char* fmt = "%4d  | %.5f           %.6f          | %.5f     %.5f        %+.5f       | %s  %5.1f\n";
    printf(fmt, cm[i], trA/N, trB/N, thA/N, thB/N, diff, perc.Data(), sigma);
    fprintf(out, fmt, cm[i], trA/N, trB/N, thA/N, thB/N, diff, perc.Data(), sigma);

    double x = cm[i];
    gTransA->SetPoint(k, x, trA/N); gTransA->SetPointError(k, 0, sqrt(trA)/N);
    gTransB->SetPoint(k, x, trB/N); gTransB->SetPointError(k, 0, sqrt(trB)/N);
    gThermA->SetPoint(k, x, thA/N); gThermA->SetPointError(k, 0, sqrt(thA)/N);
    gThermB->SetPoint(k, x, thB/N); gThermB->SetPointError(k, 0, sqrt(thB)/N);
    ++k;
  }
  const char* note = "\nWater signal = how much the thermal backscatter goes up when water is added.\n"
                     "It is a real difference if sigma is bigger than about 3.\n";
  printf("%s", note); fprintf(out, "%s", note);
  fclose(out);
  if (k == 0) return;

  // ---------- plot 1: transmission ----------
  TCanvas* c1 = new TCanvas("c1", "Transmission", 900, 600);
  c1->SetLogy(); c1->SetGrid();
  gTransA->SetTitle("Transmitted neutrons;Rock thickness (cm);neutrons per primary");
  gTransA->SetMarkerStyle(20); gTransA->SetMarkerColor(kBlue); gTransA->SetLineColor(kBlue);
  gTransB->SetMarkerStyle(21); gTransB->SetMarkerColor(kRed);  gTransB->SetLineColor(kRed);
  TMultiGraph* m1 = new TMultiGraph();
  m1->SetTitle("Transmitted neutrons;Rock thickness (cm);neutrons per primary");
  m1->Add(gTransA, "LP"); m1->Add(gTransB, "LP");
  m1->Draw("A");
  TLegend* l1 = new TLegend(0.55, 0.75, 0.88, 0.88);
  l1->AddEntry(gTransA, "rock only", "lp");
  l1->AddEntry(gTransB, "rock + 0.5 m water", "lp");
  l1->Draw();
  c1->SaveAs("sweep_transmission.png");
  c1->SaveAs("sweep_transmission.pdf");

  // ---------- plot 2: thermal backscatter ----------
  TCanvas* c2 = new TCanvas("c2", "Thermal backscatter", 900, 600);
  c2->SetGrid();
  gThermA->SetMarkerStyle(20); gThermA->SetMarkerColor(kBlue); gThermA->SetLineColor(kBlue);
  gThermB->SetMarkerStyle(21); gThermB->SetMarkerColor(kRed);  gThermB->SetLineColor(kRed);
  TMultiGraph* m2 = new TMultiGraph();
  m2->SetTitle("Thermal backscattered neutrons (< 0.5 eV);Rock thickness (cm);neutrons per primary");
  m2->Add(gThermA, "LP"); m2->Add(gThermB, "LP");
  m2->Draw("A");
  TLegend* l2 = new TLegend(0.55, 0.75, 0.88, 0.88);
  l2->AddEntry(gThermA, "rock only", "lp");
  l2->AddEntry(gThermB, "rock + 0.5 m water", "lp");
  l2->Draw();
  c2->SaveAs("sweep_thermal.png");
  c2->SaveAs("sweep_thermal.pdf");

  printf("\nSaved: sweep_summary.txt, sweep_transmission.png/.pdf, sweep_thermal.png/.pdf\n");
}
