-- query 11

-- extract sepsis cohort according to SEPSIS-3 definition 

BEGIN;

-- identify if there is suspicion of infection. If yes determine time of inf-susp! 	
\i abx-poe-list.sql
\i abx-micro-prescription.sql
\i suspicion-of-infection.sql

-- generate cohort (featuring susp-inf-time, ..)
\i cohort.sql


-- --------- VIEWS FOR SOFA AROUND INF-SUSP!
-- Generate the views which the severity scores are based on (at time of infection)
\i tbls/urine-output-infect-time.sql
\i tbls/vitals-infect-time.sql
\i tbls/gcs-infect-time.sql
\i tbls/labs-infect-time.sql 
\i tbls/blood-gas-infect-time.sql
\i tbls/blood-gas-arterial-infect-time.sql

-- additional requirements for sofa-si.sql:

\i tbls/ventilation-durations.sql

-- Severity scores around the time of suspected infection
\i tbls/sofa-si.sql 


-- --------- VIEWS FOR BASELINE SOFA 3days before INF-SUSP WINDOW! HERE ALL CASCADE (deleting depending views) when dropping view has to be commented out, otherwise sofa-si views will be deleted!
 -- Generate the views which the severity scores are based on (all 3d before window)
\i tbls_bl/bl_urine-output-infect-time.sql 
\i tbls_bl/bl_vitals-infect-time.sql
\i tbls_bl/bl_gcs-infect-time.sql
\i tbls_bl/bl_labs-infect-time.sql
\i tbls_bl/bl_blood-gas-infect-time.sql
\i tbls_bl/bl_blood-gas-arterial-infect-time.sql


-- create maximum baseline SOFA score around (-120h, -48h), CAVE: it combines all extreme values! (this is likely higher than the maximum of all scores sequentially evaluated (e.g. each hour)
-- but as this overestimation affects both sofa scores (required for the change computation) similarly, this bias should be minimal. 
-- Actually, in order to address this, in subsequent versions of this query (not public yet) the sofa score is computed on an hourly basis.

\i tbls_bl/bl_sofa-si.sql 

-- compute sofa score change (in pts):
\i sofa-change.sql

-- define sepsis case / control:
\i case-control.sql

-- extract time series for cases and controls:
\i extract-case-series.sql
\i extract-control-series.sql



--\copy ( SELECT * FROM case_vitals) To 'output/query11_case_vitals_subjectID_v5.csv' with CSV HEADER;

\copy ( SELECT * FROM case_vitals cv where cv.SysBP is not null) To 'output/query11_case_sysBP_subjectID_v5.csv' with CSV HEADER;
\copy ( SELECT * FROM case_vitals cv where cv.RespRate is not null) To 'output/query11_case_RespRate_subjectID_v5.csv' with CSV HEADER;
\copy ( SELECT * FROM case_vitals cv where cv.HeartRate is not null) To 'output/query11_case_HeartRate_subjectID_v5.csv' with CSV HEADER;

--\copy ( SELECT * FROM control_vitals) To 'output/query11_control_vitals_subjectID_v5.csv' with CSV HEADER;

\copy ( SELECT * FROM control_vitals cv where cv.SysBP is not null) To 'output/query11_control_sysBP_subjectID_v5.csv' with CSV HEADER;
\copy ( SELECT * FROM control_vitals cv where cv.RespRate is not null) To 'output/query11_control_RespRate_subjectID_v5.csv' with CSV HEADER;
\copy ( SELECT * FROM control_vitals cv where cv.HeartRate is not null) To 'output/query11_control_HeartRate_subjectID_v5.csv' with CSV HEADER;

COMMIT;
