-- SOFA CHANGE

-- compute change of SOFA points between two timepoints (e.g. sofa-si and bl_sofa-si)





DROP MATERIALIZED VIEW IF EXISTS sofachange CASCADE; 

CREATE MATERIALIZED VIEW sofachange AS   
with compute as
(
	select so.icustay_id, --bso.icustay_id,
		so.SOFA as SOFA_at_SI,
		bso.SOFA as SOFA_bl,
		(so.SOFA - bso.SOFA) as SOFA_delta

from SOFA_si so 
full join bl_SOFA_si bso
	on so.icustay_id = bso.icustay_id 
)

select si.icustay_id,
	co.SOFA_at_SI,
	co.SOFA_bl,
	co.SOFA_delta


from suspinfect_poe si  --CHANGED FROM ORIGINAL: added _poe!
 left join compute co
  on si.icustay_id = co.icustay_id
--where si.suspected_infection_time is not null
order by si.icustay_id;

