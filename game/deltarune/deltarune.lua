function _profile_create(name, id)
    new_profolder("pfpath", "chsel", name);
    new_profolder("pfpath", "ch1", name);
    new_profolder("pfpath", "ch2", name);
    new_profolder("pfpath", "ch3", name);
    new_profolder("pfpath", "ch4", name);
    new_profile(name, id);
    new_profile_data(name.."_ch0", "chsel"):
    new_profile_data(name.."_ch1", "ch1"):
    new_profile_data(name.."_ch2", "ch2"):
    new_profile_data(name.."_ch3", "ch3"):
    new_profile_data(name.."_ch4", "ch4"):
end